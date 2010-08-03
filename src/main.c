/*
 * ngfd - Non-graphic feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation.
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "log.h"
#include "context.h"
#include "dbus-if.h"
#include "profile.h"
#include "settings.h"
#include "session.h"
#include "volume-controller.h"

static gboolean _request_manager_create         (Context *context);
static void     _request_manager_destroy        (Context *context);
static DBusHandlerResult _dbus_event            (DBusConnection * connection, DBusMessage * message,
	      void *userdata);

static DBusConnection*
_get_dbus_connection (DBusBusType bus_type)
{
    DBusConnection *bus = NULL;
    DBusError       error;

    dbus_error_init (&error);
    bus = dbus_bus_get (bus_type, &error);
    if (bus == NULL) {
        NGF_LOG_WARNING ("%s >> failed to get %s bus: %s", bus_type == DBUS_BUS_SYSTEM ? "system" : "session", error.message);
        dbus_error_free (&error);
        return NULL;
    }

    return bus;
}

static DBusHandlerResult _dbus_event (DBusConnection * connection, DBusMessage * message,
	      void *userdata)
{
    Context *context = (Context *) userdata;
    DBusError error = DBUS_ERROR_INIT;
    gchar *component = NULL;
    gchar *s1 = NULL;
    gchar *s2 = NULL;

    if (dbus_message_is_signal
	    (message, "org.freedesktop.DBus", "NameOwnerChanged")) {
        if (!dbus_message_get_args
            (message, &error,
            DBUS_TYPE_STRING, &component,
            DBUS_TYPE_STRING, &s1,
            DBUS_TYPE_STRING, &s2,
            DBUS_TYPE_INVALID)) {
            if (error.message)
                NGF_LOG_WARNING ("D-Bus error: %s",error.message);
        } else {
            if (component && g_str_equal (component, "org.freedesktop.ohm")) {
                NGF_LOG_INFO ("Ohmd restarted, stopping all requests");
                stop_handler (context, 0);
            }
        }
        dbus_error_free (&error);
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int
context_create (Context **context)
{
    Context *c = NULL;
    struct sigaction act;

    if ((c = g_try_malloc0 (sizeof (Context))) == NULL) {
        NGF_LOG_WARNING ("Failed to allocate memory for context!");
        return FALSE;
    }

    if ((c->loop = g_main_loop_new (NULL, 0)) == NULL) {
        NGF_LOG_WARNING ("Failed to create the GLib mainloop!");
        return FALSE;
    }

    if ((c->system_bus = _get_dbus_connection (DBUS_BUS_SYSTEM)) == NULL)
        return FALSE;

    dbus_connection_setup_with_g_main (c->system_bus, NULL);

    /* setup the interface */

    if (!dbus_if_create (c)) {
        NGF_LOG_WARNING ("Failed to create D-Bus interface!");
        return FALSE;
    }

    /* setup the backends */

    if (!volume_controller_create (c)) {
        NGF_LOG_WARNING ("%s >> failed to create volume control.", __FUNCTION__);
    }

    if (!profile_create (c)) {
        NGF_LOG_WARNING ("Failed to create profile tracking!");
        return FALSE;
    }

    if (!tone_mapper_create (c)) {
        NGF_LOG_WARNING ("Failed to create tone mapper!");
        return FALSE;
    }

    if ((c->audio = audio_create ()) == NULL) {
        NGF_LOG_WARNING ("Failed to create Pulseaudio backend!");
        return FALSE;
    }

    if ((c->vibrator = vibrator_create ()) == NULL) {
        NGF_LOG_WARNING ("Failed to create Immersion backend!");
        return FALSE;
    }

    /* create the hash tables to hold definitions and events */

    if (!_request_manager_create (c)) {
        NGF_LOG_WARNING ("Failed to create request manager!");
        return FALSE;
    }

    if (!load_settings (c)) {
        NGF_LOG_WARNING ("Failed to load settings!");
        return FALSE;
    }

    /* hook cleanup function to ohmd restarts */

    dbus_bus_add_match (c->system_bus,
    "type='signal',sender='org.freedesktop.DBus',member='NameOwnerChanged',arg0='org.freedesktop.ohm'",
	NULL);

    dbus_connection_add_filter (c->system_bus, _dbus_event, c, NULL);

    /* hook up the session watcher */

    if (!session_create (c)) {
        NGF_LOG_WARNING ("%s >> failed to setup session watcher.", __FUNCTION__);
        return FALSE;
    }

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = log_signal;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    *context = c;
    return TRUE;
}

void
context_destroy (Context *context)
{
    dbus_connection_remove_filter (context->system_bus, _dbus_event, context);
    dbus_if_destroy           (context);
    profile_destroy           (context);
    tone_mapper_destroy       (context);
    session_destroy           (context);
    volume_controller_destroy (context);

    if (context->session_bus) {
        dbus_connection_unref (context->session_bus);
        context->session_bus = NULL;
    }

    if (context->system_bus) {
        dbus_connection_unref (context->system_bus);
        context->system_bus = NULL;
    }

    if (context->vibrator) {
        vibrator_destroy (context->vibrator);
        context->vibrator = NULL;
    }

    if (context->audio) {
        audio_destroy (context->audio);
        context->audio = NULL;
    }

    _request_manager_destroy (context);

    if (context->loop) {
        g_main_loop_unref (context->loop);
        context->loop = NULL;
    }

    sound_path_array_free        (context->sounds);
    vibration_pattern_array_free (context->patterns);
    volume_array_free            (context->volumes);
    g_free                       (context->patterns_path);

    g_free (context);
}

static gboolean
_request_manager_create (Context *context)
{
    context->definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) definition_free);
    if (context->definitions == NULL)
        return FALSE;

    context->events = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) event_free);
    if (context->events == NULL)
        return FALSE;

    return TRUE;
}

static void
_request_manager_destroy (Context *context)
{
    if (context->events) {
        g_hash_table_destroy (context->events);
        context->events = NULL;
    }

    if (context->definitions) {
        g_hash_table_destroy (context->definitions);
        context->definitions = NULL;
    }
}

static gboolean
parse_cmdline (int argc, char **argv)
{
    int opt, opt_index;
    int level = LOG_LEVEL_NONE;

    static struct option long_opts[] = {
        { "verbose", 0, 0, 'v' },
        { 0, 0, 0, 0 }
    };

    while ((opt = getopt_long (argc, argv, "v", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 'v':
                if (level)
                    level--;
                break;

            default:
                break;
        }
    }

    log_set_level (level);

    return TRUE;
}

int
main (int argc, char *argv[])
{
    Context *context = NULL;

    if (!parse_cmdline (argc, argv))
        return 1;

    if (!context_create (&context))
        return 1;

    g_main_loop_run (context->loop);
    context_destroy (context);

    return 0;
}
