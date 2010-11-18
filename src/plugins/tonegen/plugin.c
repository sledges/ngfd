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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <ngf/plugin.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#define LOG_CAT        "tonegen: "
#define TONEGEN_NAME   "com.Nokia.Telephony.Tones"
#define TONEGEN_PATH   "/com/Nokia/Telephony/Tones"
#define TONEGEN_IFACE  "com.Nokia.Telephony.Tones"
#define TONEGEN_START  "StartEventTone"
#define TONEGEN_STOP   "StopTone"

N_PLUGIN_NAME        ("tonegen")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_DESCRIPTION ("Tone generator plugin for knocking sound")

static DBusConnection *system_bus = NULL;

static gboolean
call_dbus_method (DBusConnection *bus, DBusMessage *msg)
{
    if (!dbus_connection_send (bus, msg, 0)) {
        N_WARNING (LOG_CAT "failed to send DBus message %s",
            dbus_message_get_member (msg), dbus_message_get_interface (msg));

        return FALSE;
    }

    return TRUE;
}

static gboolean
tone_generator_toggle (DBusConnection *bus, guint pattern, gint volume,
                       gboolean activate)
{
    DBusMessage   *msg    = NULL;
    gboolean       ret    = FALSE;
    dbus_uint32_t  length = 0;
    const char    *cmd    = activate ? TONEGEN_START : TONEGEN_STOP;

    if (bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (TONEGEN_NAME,
                                        TONEGEN_PATH,
                                        TONEGEN_IFACE,
                                        cmd);

    if (msg == NULL)
        return FALSE;

    if (activate) {
        N_DEBUG (LOG_CAT "activating LED pattern %u with volume %d",
            pattern, volume);

        ret = dbus_message_append_args (msg,
            DBUS_TYPE_UINT32, &pattern,
            DBUS_TYPE_INT32, &volume,
            DBUS_TYPE_UINT32, &length,
            DBUS_TYPE_INVALID);

        if (!ret) {
            dbus_message_unref (msg);
            return FALSE;
        }
    }
    else {
        N_DEBUG (LOG_CAT "deactivating LED pattern");
    }

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);
    return ret;
}

static int
tonegen_sink_initialize (NSinkInterface *iface)
{
    (void) iface;

    DBusError error;

    dbus_error_init (&error);
    system_bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
    if (!system_bus) {
        N_WARNING (LOG_CAT "failed to get system bus: %s", error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    dbus_connection_setup_with_g_main (system_bus, NULL);

    return TRUE;
}

static void
tonegen_sink_shutdown (NSinkInterface *iface)
{
    (void) iface;

    if (system_bus) {
        dbus_connection_unref (system_bus);
        system_bus = NULL;
    }
}

static int
tonegen_sink_can_handle (NSinkInterface *iface, NRequest *request)
{
    (void) iface;

    NProplist *props = (NProplist*) n_request_get_properties (request);

    if (n_proplist_has_key (props, "tonegen.pattern"))
        return TRUE;

    return FALSE;
}

static int
tonegen_sink_prepare (NSinkInterface *iface, NRequest *request)
{
    n_sink_interface_synchronize (iface, request);
    return TRUE;
}

static int
tonegen_sink_play (NSinkInterface *iface, NRequest *request)
{
    (void) iface;

    NProplist *props   = (NProplist*) n_request_get_properties (request);
    guint      pattern = 0;
    gint       volume  = 0;

    pattern = (guint) n_proplist_get_int (props, "tonegen.pattern");
    volume  = n_proplist_get_int  (props, "tonegen.volume");

    tone_generator_toggle (system_bus, pattern, volume, TRUE);

    return TRUE;
}

static int
tonegen_sink_pause (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;

    return TRUE;
}

static void
tonegen_sink_stop (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;

    tone_generator_toggle (system_bus, 0, 0, FALSE);
}

N_PLUGIN_LOAD (plugin)
{
    static const NSinkInterfaceDecl decl = {
        .name       = "tonegen",
        .initialize = tonegen_sink_initialize,
        .shutdown   = tonegen_sink_shutdown,
        .can_handle = tonegen_sink_can_handle,
        .prepare    = tonegen_sink_prepare,
        .play       = tonegen_sink_play,
        .pause      = tonegen_sink_pause,
        .stop       = tonegen_sink_stop
    };

    n_plugin_register_sink (plugin, &decl);

    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    (void) plugin;
}
