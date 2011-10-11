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
 
#include <string.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <mce/dbus-names.h>

#include <ngf/plugin.h>
#include <ngf/context.h>

#define LOG_CAT             "mce: "
#define DISPLAY_STATUS_KEY "display_status"

N_PLUGIN_NAME        ("mce")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_DESCRIPTION ("MCE plugin for querying screen status")

static DBusConnection *system_bus = NULL;

static gboolean
initialize_system_bus ()
{
    DBusError error;

    dbus_error_init (&error);
    system_bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);

    if (dbus_error_is_set (&error)) {
        N_WARNING (LOG_CAT "failed to open connection to system bus: %s",
            error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    dbus_connection_setup_with_g_main (system_bus, NULL);

    return TRUE;
}

static void
shutdown_system_bus ()
{
    if (system_bus != NULL) {
        dbus_connection_unref (system_bus);
        system_bus = NULL;
    }
}

static void
store_display_status (NContext *context, const char *value)
{
    g_assert (context != NULL);

    NValue *v;

    N_DEBUG (LOG_CAT "display status changed to: %s", value);

    v = n_value_new ();
    n_value_set_string (v, value);
    n_context_set_value (context, DISPLAY_STATUS_KEY, v);
}

static DBusHandlerResult
receive_display_status_signal (DBusConnection *connection, DBusMessage *msg, void *user_data)
{
    (void) connection;

    NContext *context = (NContext*) user_data;
    const char *value;
 
    if (dbus_message_is_signal (msg, MCE_SIGNAL_IF, MCE_DISPLAY_SIG)) {
        N_INFO (LOG_CAT "received display status signal");
        if (dbus_message_get_args (msg, NULL, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
            store_display_status (context, value);
        }
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
follow_display_status_signal (NContext *context)
{
    DBusError error;

    dbus_error_init (&error);
    dbus_bus_add_match (system_bus,
                        "interface=" MCE_SIGNAL_IF ","
                        "path=" MCE_SIGNAL_PATH ","
                        "member=" MCE_DISPLAY_SIG,
                        &error);

    if (dbus_error_is_set (&error)) {
        N_WARNING (LOG_CAT "failed to add watch: %s",
            error.message);
        dbus_error_free (&error);
        return;
    }

    if (!dbus_connection_add_filter (system_bus, receive_display_status_signal, context, NULL)) {
        N_WARNING (LOG_CAT "failed to add filter");
        return;
    }
}

static void
complete_display_status (DBusPendingCall *pending, void *user_data)
{
    DBusMessage *msg;
    const char *value;
    NContext *context = (NContext*) user_data;

    msg = dbus_pending_call_steal_reply (pending);

    if (msg == NULL || dbus_message_get_type (msg) == DBUS_MESSAGE_TYPE_ERROR) {
        N_WARNING (LOG_CAT "failed to MCE display status reply");
        goto fail_msg;
    }

    if (!dbus_message_get_args (msg, NULL, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
        N_WARNING (LOG_CAT "invalid reply to MCE display status");
        goto fail_msg;
    }

    store_display_status (context, value);

fail_msg:
    dbus_message_unref (msg);
    dbus_pending_call_unref (pending);
}

static gboolean
request_display_status (NContext *context)
{
    DBusMessage *msg;
    DBusPendingCall *pending;

    N_INFO (LOG_CAT "requesting display status");

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH,
        MCE_REQUEST_IF, MCE_DISPLAY_STATUS_GET);

    if (msg == NULL) {
        N_WARNING (LOG_CAT "failed to create MCE DBus request");
        return FALSE;
    }

    if (!dbus_connection_send_with_reply (system_bus, msg, &pending, -1)) {
        N_WARNING (LOG_CAT "faile to send display status message");
        dbus_message_unref (msg);
        return FALSE;
    }

    dbus_pending_call_set_notify (pending, complete_display_status, context, NULL);
    dbus_message_unref (msg);

    return TRUE;
}

N_PLUGIN_LOAD (plugin)
{
    NCore    *core    = n_plugin_get_core (plugin);
    NContext *context = n_core_get_context (core);

    core = n_plugin_get_core (plugin);

    if (initialize_system_bus ()) {
        follow_display_status_signal (context);
        request_display_status (context);
    }

    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    (void) plugin;

    shutdown_system_bus ();
}
