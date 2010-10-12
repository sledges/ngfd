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
    guint prevent_display_blank_id;
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
prevent_display_blank (gpointer userdata)
{
    (void) userdata;
    DBusMessage    *msg = NULL;
    gboolean        ret = FALSE;
    
    if (bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_PREVENT_BLANK_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);

    return ret;
}

gboolean
backlight_display_on (MceData *data)
{
    (void) data;
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

gboolean
backlight_prevent_blank (MceData *data)
{
    if (bus == NULL)
        return FALSE;

    if (data->prevent_display_blank_id > 0)
        g_source_remove (data->prevent_display_blank_id);

    data->prevent_display_blank_id = g_timeout_add_seconds (DISPLAY_BLANK_TIMEOUT, prevent_display_blank, bus);

    return (gboolean) data->prevent_display_blank_id;
}

gboolean
backlight_cancel_prevent_blank (MceData *data)
{
    (void) data;
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (bus == NULL)
        return FALSE;

    if (data->prevent_display_blank_id > 0) {
        g_source_remove (data->prevent_display_blank_id);
        data->prevent_display_blank_id = 0;
    }

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_CANCEL_PREVENT_BLANK_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);

    return ret;
}
/*
static gboolean
toggle_pattern (DBusConnection *bus, const char *pattern, gboolean activate)
{
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (bus == NULL || pattern == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF,
        activate ? MCE_ACTIVATE_LED_PATTERN : MCE_DEACTIVATE_LED_PATTERN);

    if (msg == NULL)
        return FALSE;

    if (!dbus_message_append_args (msg, DBUS_TYPE_STRING, &pattern, DBUS_TYPE_INVALID)) {
        dbus_message_unref (msg);
        return FALSE;
    }

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);

    if (ret)
        N_DEBUG ("%s >> led pattern %s %s.", __FUNCTION__, pattern, activate ? "activated" : "deactivated");

    return ret;
}*/

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
    (void) request;
    
    N_DEBUG (LOG_CAT "sink can_handle %s",n_request_get_name(request));
    n_proplist_dump(n_request_get_properties(request));
    
    return TRUE;
}

static int
mce_sink_prepare (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;
    
    N_DEBUG (LOG_CAT "sink prepare");
    return TRUE;
}

static int
mce_sink_play (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;
    
    N_DEBUG (LOG_CAT "sink play");
    return TRUE;
}

static void
mce_sink_stop (NSinkInterface *iface, NRequest *request)
{
    (void) iface;
    (void) request;
    
    N_DEBUG (LOG_CAT "sink stop");
}

N_PLUGIN_LOAD (plugin)
{
    N_DEBUG (LOG_CAT "plugin load");

    static const NSinkInterfaceDecl decl = {
        .name       = "mce",
        .initialize = mce_sink_initialize,
        .shutdown   = mce_sink_shutdown,
        .can_handle = mce_sink_can_handle,
        .prepare    = mce_sink_prepare,
        .play       = mce_sink_play,
        .stop       = mce_sink_stop
    };

    n_plugin_register_sink (plugin, &decl);

    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    (void) plugin;

    N_DEBUG (LOG_CAT "plugin unload");
}
