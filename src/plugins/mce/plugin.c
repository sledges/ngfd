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
 
#include <ngf/plugin.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <mce/dbus-names.h>

#define MCE_KEY "plugin.mce.data"
#define LOG_CAT "mce: "
#define DISPLAY_BLANK_TIMEOUT 50

N_PLUGIN_NAME        ("mce")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_AUTHOR      ("Harri Mahonen <ext-harri.mahonen@nokia.com>")
N_PLUGIN_DESCRIPTION ("MCE plugin for handling backlight and led actions")

typedef struct _MceData
{
    NRequest       *request;
    NSinkInterface *iface;
} MceData;

DBusConnection *bus = NULL;

static gboolean
call_dbus_method (DBusConnection *bus, DBusMessage *msg)
{
    if (!dbus_connection_send (bus, msg, 0)) {
        N_WARNING ("Failed to send DBus message %s to interface %s", dbus_message_get_member (msg), dbus_message_get_interface (msg));
        return FALSE;
    }

    return TRUE;
}

static gboolean
backlight_on (void)
{
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DISPLAY_ON_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);

    return ret;
}

static int
mce_sink_initialize (NSinkInterface *iface)
{
    (void) iface;
    DBusError       error;

    dbus_error_init (&error);
    bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
    if (bus == NULL) {
        N_WARNING ("%s >> failed to get system bus: %s",error.message);
        dbus_error_free (&error);
        return FALSE;
    }
    
    return TRUE;
}

static void
mce_sink_shutdown (NSinkInterface *iface)
{
    (void) iface;
}

static int
mce_sink_can_handle (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    const NProplist *props = n_request_get_properties (request);

    if (n_proplist_has_key (props, "mce.backlight_on") || n_proplist_has_key (props, "mce.led_pattern")) {
        return TRUE;
    }

    return FALSE;
}

static int
mce_sink_prepare (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;
    
    MceData *data = g_slice_new0 (MceData);

    data->request    = request;
    data->iface      = iface;

    n_request_store_data (request, MCE_KEY, data);
    n_sink_interface_synchronize (iface, request);
    
    return TRUE;
}

static int
mce_sink_play (NSinkInterface *iface, NRequest *request)
{
    const NProplist *props = n_request_get_properties (request);
    (void) iface;

    MceData *data = (MceData*) n_request_get_data (request, MCE_KEY);
    g_assert (data != NULL);

    if (n_proplist_get_bool (props, "mce.backlight_on"))
        backlight_on ();

    n_sink_interface_complete (data->iface, data->request);
    
    return TRUE;
}

static int
mce_sink_pause (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;

    return TRUE;
}

static void
mce_sink_stop (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;
}

N_PLUGIN_LOAD (plugin)
{
    static const NSinkInterfaceDecl decl = {
        .name       = "mce",
        .initialize = mce_sink_initialize,
        .shutdown   = mce_sink_shutdown,
        .can_handle = mce_sink_can_handle,
        .prepare    = mce_sink_prepare,
        .play       = mce_sink_play,
        .pause      = mce_sink_pause,
        .stop       = mce_sink_stop
    };

    n_plugin_register_sink (plugin, &decl);

    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    (void) plugin;
}
