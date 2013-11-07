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
#include <ngf/plugin.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <mce/dbus-names.h>

#define MCE_KEY "plugin.mce.data"
#define LOG_CAT "mce: "
#define DISPLAY_BLANK_TIMEOUT 50

N_PLUGIN_NAME        ("mce")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_DESCRIPTION ("MCE plugin for handling backlight and led actions")

typedef struct _MceData
{
    NRequest       *request;
    NSinkInterface *iface;
    gchar          *pattern;
} MceData;

typedef struct _MceStateData
{
    DBusConnection *bus;
    GList *active_requests;
    guint pattern_timeout;
} MceStateData;

static MceStateData state;

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

    if (state.bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DISPLAY_ON_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (state.bus, msg);
    dbus_message_unref (msg);

    return ret;
}

static gboolean
toggle_pattern (const char *pattern, gboolean activate)
{
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (state.bus == NULL || pattern == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF,
        activate ? MCE_ACTIVATE_LED_PATTERN : MCE_DEACTIVATE_LED_PATTERN);

    if (msg == NULL)
        return FALSE;

    if (!dbus_message_append_args (msg, DBUS_TYPE_STRING, &pattern, DBUS_TYPE_INVALID)) {
        dbus_message_unref (msg);
        return FALSE;
    }

    ret = call_dbus_method (state.bus, msg);
    dbus_message_unref (msg);

    if (ret)
        N_DEBUG ("%s >> led pattern %s %s.", __FUNCTION__, pattern, activate ? "activated" : "deactivated");

    return ret;
}

static void
mce_playback_active_request_done (gpointer pdata, gpointer userdata)
{
    (void) userdata;

    MceData *data = (MceData *) pdata;

    n_sink_interface_complete (data->iface, data->request);
}

static DBusHandlerResult
mce_bus_filter (DBusConnection *connection, DBusMessage *message, void *user_data)
{
    (void) connection;
    (void) user_data;
    int clear_requests = FALSE;

    if (g_list_first (state.active_requests) == NULL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (dbus_message_is_signal (message, MCE_SIGNAL_IF, MCE_INACTIVITY_SIG)) {
        DBusError error;
        int inactive = FALSE;

        dbus_error_init (&error);
        if (!dbus_message_get_args (message, &error, DBUS_TYPE_BOOLEAN, &inactive, DBUS_TYPE_INVALID)) {
            N_WARNING (LOG_CAT "failed to get arguments: %s",
                error.message);
            dbus_error_free (&error);
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        /* Always clear patterns if there's activity */
        if (!inactive) {
            N_DEBUG(LOG_CAT "Clearing mce led patterns due to activity");
            clear_requests = TRUE;
        }

    } else if (dbus_message_is_signal (message, MCE_SIGNAL_IF, MCE_DISPLAY_SIG)) {
        DBusError error;
        char *status = NULL;

        dbus_error_init(&error);
        if (!dbus_message_get_args (message, &error, DBUS_TYPE_STRING, &status, DBUS_TYPE_INVALID)) {
            N_WARNING (LOG_CAT "failed to get arguments: %s",
                error.message);
            dbus_error_free (&error);
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        if (status == NULL)
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

        /* Clear patterns when screen goes on, unless there was a pattern added just before.
         * This is an suboptimal way to detect race between activating a pattern and turning
         * the screen on without user interaction (eg. incoming SMS, call might do this).
         * This means we don't clear requests that come in just before the user happens to
         * turn the display on. This is not a huge problem since the user probably turned
         * the display on to interact and thus it will be cleared on activity.
         */
        if (g_str_equal (status, "on") && state.pattern_timeout == 0) {
            N_DEBUG (LOG_CAT "Clearing mce led patterns due to display on");
            clear_requests = TRUE;
        }

    } else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (clear_requests) {
        g_list_foreach (state.active_requests, mce_playback_active_request_done, NULL);
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static int
mce_sink_initialize (NSinkInterface *iface)
{
    (void) iface;
    DBusError error;

    memset (&state, 0, sizeof(MceStateData));

    dbus_error_init (&error);
    state.bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
    if (state.bus == NULL) {
        N_WARNING ("%s >> failed to get system bus: %s",error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    dbus_connection_setup_with_g_main (state.bus, NULL);

    dbus_bus_add_match (state.bus,
                        "interface=" MCE_SIGNAL_IF ","
                        "path=" MCE_SIGNAL_PATH ","
                        "member=" MCE_DISPLAY_SIG,
                        &error);

    if (dbus_error_is_set (&error)) {
        N_WARNING (LOG_CAT "failed to add watch: %s",
            error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    if (!dbus_connection_add_filter (state.bus, mce_bus_filter, &state, NULL)) {
        N_WARNING (LOG_CAT "failed to add filter");
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

static gboolean
mce_last_pattern_timeout (gpointer userdata)
{
    (void) userdata;

    state.pattern_timeout = 0;
    return FALSE;
}

static int
mce_sink_play (NSinkInterface *iface, NRequest *request)
{
    const NProplist *props = n_request_get_properties (request);
    (void) iface;
    const gchar *pattern = NULL;

    MceData *data = (MceData*) n_request_get_data (request, MCE_KEY);
    g_assert (data != NULL);

    if (n_proplist_get_bool (props, "mce.backlight_on"))
        backlight_on ();

    pattern = n_proplist_get_string (props, "mce.led_pattern");
    if (pattern != NULL) {
        data->pattern = g_strdup (pattern);
        if (!toggle_pattern (pattern, TRUE)) {
            g_free (data->pattern);
            data->pattern = NULL;
        }
    }

    if (state.pattern_timeout != 0) {
        N_DEBUG(LOG_CAT "restarting led patterns timeout");
        g_source_remove (state.pattern_timeout);
    }

    state.pattern_timeout = g_timeout_add (2000, mce_last_pattern_timeout, data);
    state.active_requests = g_list_append (state.active_requests, (gpointer)data);

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

    MceData *data = (MceData*) n_request_get_data (request, MCE_KEY);
    g_assert (data != NULL);

    state.active_requests = g_list_remove (state.active_requests, (gpointer)data);

    if (data->pattern) {
        toggle_pattern (data->pattern, FALSE);
        g_free (data->pattern);
        data->pattern = NULL;
    }

    g_slice_free (MceData, data);
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
