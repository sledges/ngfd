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

#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "log.h"
#include "volume-controller.h"

#define PULSE_DBUS_ADDRESS  "unix:path=/var/run/pulse/dbus-socket"
#define PULSE_CORE_PATH     "/org/pulseaudio/core1"
#define STREAM_RESTORE_PATH "/org/pulseaudio/stream_restore1"
#define STREAM_RESTORE_IF   "org.PulseAudio.Ext.StreamRestore1"

#define ADD_ENTRY_METHOD    "AddEntry"
#define DISCONNECTED_SIG    "Disconnected"
#define RETRY_TIMEOUT        2
#define VOLUME_SCALE_VALUE   50000 /* 65536 */

static gboolean          retry_timeout_cb           (gpointer userdata);
static DBusHandlerResult filter_cb                  (DBusConnection *connection, DBusMessage *msg, void *data);
static void              append_volume              (DBusMessageIter *iter, guint volume);
static gboolean          add_entry                  (Context *context, const char *role, guint volume);
static void              process_queued_ops         (Context *context);

static gboolean          connect_to_pulseaudio      (Context *context);
static void              disconnect_from_pulseaudio (Context *context);



static gboolean
retry_timeout_cb (gpointer userdata)
{
    Context *context = (Context*) userdata;

    disconnect_from_pulseaudio (context);
    if (!connect_to_pulseaudio (context)) {
        N_WARNING ("%s >> failed to reconnect, trying again in %d seconds", __FUNCTION__, RETRY_TIMEOUT);
        context->volume_retry_id = g_timeout_add_seconds (RETRY_TIMEOUT, retry_timeout_cb, context);
    }
    else {
        N_DEBUG ("%s >> reconnected to pulseaudio.", __FUNCTION__);
        process_queued_ops (context);
    }

    return FALSE;
}

static DBusHandlerResult
filter_cb (DBusConnection *connection, DBusMessage *msg, void *data)
{
    Context *context = (Context*) data;

    (void) connection;

    if (dbus_message_has_interface (msg, DBUS_INTERFACE_LOCAL) &&
        dbus_message_has_path      (msg, DBUS_PATH_LOCAL) &&
        dbus_message_has_member    (msg, DISCONNECTED_SIG))
    {
        N_DEBUG ("%s >> pulseaudio disconnected, reconnecting in %d seconds", __FUNCTION__, RETRY_TIMEOUT);

        disconnect_from_pulseaudio (context);
        context->volume_retry_id = g_timeout_add_seconds (RETRY_TIMEOUT, retry_timeout_cb, context);
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
append_volume (DBusMessageIter *iter, guint volume)
{
    DBusMessageIter array, str;

    dbus_uint32_t pos = 0;
    dbus_uint32_t vol = volume;

    dbus_message_iter_open_container  (iter, DBUS_TYPE_ARRAY, "(uu)", &array);
    dbus_message_iter_open_container  (&array, DBUS_TYPE_STRUCT, NULL, &str);

    dbus_message_iter_append_basic    (&str, DBUS_TYPE_UINT32, &pos);
    dbus_message_iter_append_basic    (&str, DBUS_TYPE_UINT32, &vol);

    dbus_message_iter_close_container (&array, &str);
    dbus_message_iter_close_container (iter, &array);
}

static gboolean
add_entry (Context *context, const char *role, guint volume)
{
    DBusMessage     *msg     = NULL;
    DBusMessage     *reply   = NULL;
    const char      *empty   = "";
    gboolean         success = FALSE;
    dbus_bool_t      muted   = FALSE;
    dbus_bool_t      apply   = TRUE;
    dbus_uint32_t    vol     = 0;
    DBusMessageIter  iter;
    DBusError        error;

    if (!context->volume_bus || !role)
        return FALSE;

    /* convert the volume from 0-100 to PA_VOLUME_NORM range */
    vol = ((gdouble) (volume > 100 ? 100 : volume) / 100.0) * VOLUME_SCALE_VALUE;

    dbus_error_init (&error);
    msg = dbus_message_new_method_call (0, STREAM_RESTORE_PATH, STREAM_RESTORE_IF, ADD_ENTRY_METHOD);
    if (msg == NULL)
        goto done;

    dbus_message_iter_init_append  (msg, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &role);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &empty);
    append_volume                  (&iter, vol);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &muted);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &apply);

    reply = dbus_connection_send_with_reply_and_block (context->volume_bus, msg, -1, &error);

    if (!reply) {
        if (dbus_error_is_set (&error)) {
            N_WARNING ("%s >> error: %s", __FUNCTION__, error.message);
        }

        goto done;
    }

    N_DEBUG ("%s >> volume for role %s set to %d", __FUNCTION__, role, vol);
    success = TRUE;

done:
    dbus_error_free (&error);

    if (reply) dbus_message_unref (reply);
    if (msg)   dbus_message_unref (msg);

    return success;
}

static void
process_queued_ops (Context *context)
{
    Volume *queued_volume = NULL;

    if (!context)
        return;

    while ((queued_volume = g_queue_pop_head (context->volume_queue)) != NULL) {
        N_DEBUG ("%s >> processing queued volume (role=%s, volume=%d)", __FUNCTION__, queued_volume->role, queued_volume->level);
        add_entry (context, queued_volume->role, queued_volume->level);
    }
}

static gboolean
connect_to_pulseaudio (Context *context)
{
    const char *pulse_address = NULL;
    DBusError   error;

    if ((pulse_address = getenv ("PULSE_DBUS_SERVER")) == NULL)
        pulse_address = PULSE_DBUS_ADDRESS;

    dbus_error_init (&error);
    context->volume_bus = dbus_connection_open (pulse_address, &error);

    if (dbus_error_is_set (&error)) {
        N_WARNING ("%s >> failed to open connection to pulseaudio: %s", __FUNCTION__, error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    dbus_connection_setup_with_g_main (context->volume_bus, NULL);

    if (!dbus_connection_add_filter (context->volume_bus, filter_cb, context, NULL)) {
        N_WARNING ("%s >> failed to add filter", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

static void
disconnect_from_pulseaudio (Context *context)
{
    if (!context)
        return;

    if (context->volume_retry_id > 0) {
        g_source_remove (context->volume_retry_id);
        context->volume_retry_id = 0;
    }

    if (context->volume_bus) {
        dbus_connection_unref (context->volume_bus);
        context->volume_bus = NULL;
    }
}

int
volume_controller_create (Context *context)
{
    if (!context)
        return FALSE;

    if ((context->volume_queue = g_queue_new ()) == NULL)
        return FALSE;

    if (!connect_to_pulseaudio (context)) {
        N_DEBUG ("%s >> failed to connect to Pulseaudio DBus, reconnecting in %d seconds.", __FUNCTION__, RETRY_TIMEOUT);
        context->volume_retry_id = g_timeout_add_seconds (RETRY_TIMEOUT, retry_timeout_cb, context);
    }

    return TRUE;
}

void
volume_controller_destroy (Context *context)
{
    if (!context)
        return;

    disconnect_from_pulseaudio (context);

    if (context->volume_queue) {
        g_queue_free (context->volume_queue);
        context->volume_queue = NULL;
    }
}

int
volume_controller_update (Context *context, Volume *volume)
{
    if (!context || !volume)
        return FALSE;

    if (!volume->role)
        return FALSE;

    if (!context->volume_bus) {
        N_DEBUG ("%s >> volume controller not ready, queueing op.", __FUNCTION__);
        g_queue_push_tail (context->volume_queue, volume);
        return TRUE;
    }

    return add_entry (context, volume->role, volume->level);
}

int
volume_controller_update_all (Context *context)
{
    Volume **i = NULL;

    if (!context || (context && !context->volumes))
        return FALSE;

    for (i = context->volumes; *i; ++i)
        volume_controller_update (context, *i);

    return TRUE;
}
