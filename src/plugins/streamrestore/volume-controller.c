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

#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <ngf/log.h>

#define LOG_CAT             "stream-restore: "

#define PULSE_DBUS_ADDRESS  "unix:path=/var/run/pulse/dbus-socket"
#define PULSE_CORE_PATH     "/org/pulseaudio/core1"
#define STREAM_RESTORE_PATH "/org/pulseaudio/stream_restore1"
#define STREAM_RESTORE_IF   "org.PulseAudio.Ext.StreamRestore1"

#define ADD_ENTRY_METHOD    "AddEntry"
#define DISCONNECTED_SIG    "Disconnected"
#define RETRY_TIMEOUT        2
#define VOLUME_SCALE_VALUE   65536

typedef struct _QueueItem
{
    gchar *role;
    int    volume;
} QueueItem;

static GQueue         *volume_queue    = NULL;
static DBusConnection *volume_bus      = NULL;
static guint           volume_retry_id = 0;

static gboolean          retry_timeout_cb           (gpointer userdata);
static DBusHandlerResult filter_cb                  (DBusConnection *connection, DBusMessage *msg, void *data);
static void              append_volume              (DBusMessageIter *iter, guint volume);
static gboolean          add_entry                  (const char *role, guint volume);
static void              process_queued_ops         ();
static gboolean          connect_to_pulseaudio      ();
static void              disconnect_from_pulseaudio ();



static gboolean
retry_timeout_cb (gpointer userdata)
{
    (void) userdata;

    disconnect_from_pulseaudio ();
    if (!connect_to_pulseaudio ()) {
        N_WARNING (LOG_CAT "failed to reconnect, trying again in %d seconds",
            RETRY_TIMEOUT);

        volume_retry_id = g_timeout_add_seconds (RETRY_TIMEOUT,
            retry_timeout_cb, NULL);
    }
    else {
        N_DEBUG (LOG_CAT "reconnected to pulseaudio.");
        process_queued_ops ();
    }

    return FALSE;
}

static DBusHandlerResult
filter_cb (DBusConnection *connection, DBusMessage *msg, void *data)
{
    (void) connection;
    (void) data;

    if (dbus_message_has_interface (msg, DBUS_INTERFACE_LOCAL) &&
        dbus_message_has_path      (msg, DBUS_PATH_LOCAL) &&
        dbus_message_has_member    (msg, DISCONNECTED_SIG))
    {
        N_DEBUG (LOG_CAT "pulseaudio disconnected, reconnecting in %d seconds",
            RETRY_TIMEOUT);

        disconnect_from_pulseaudio ();
        volume_retry_id = g_timeout_add_seconds (RETRY_TIMEOUT,
            retry_timeout_cb, NULL);
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
add_entry (const char *role, guint volume)
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

    if (!volume_bus || !role)
        return FALSE;

    /* convert the volume from 0-100 to PA_VOLUME_NORM range */
    vol = ((gdouble) (volume > 100 ? 100 : volume) / 100.0) * VOLUME_SCALE_VALUE;

    dbus_error_init (&error);
    msg = dbus_message_new_method_call (0, STREAM_RESTORE_PATH,
        STREAM_RESTORE_IF, ADD_ENTRY_METHOD);

    if (msg == NULL)
        goto done;

    dbus_message_iter_init_append  (msg, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &role);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &empty);
    append_volume                  (&iter, vol);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &muted);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &apply);

    reply = dbus_connection_send_with_reply_and_block (volume_bus,
        msg, -1, &error);

    if (!reply) {
        if (dbus_error_is_set (&error)) {
            N_WARNING (LOG_CAT "failed to update volume role '%s': %s",
                role, error.message);
        }

        goto done;
    }

    N_DEBUG (LOG_CAT "volume for role '%s' set to %d", role, vol);
    success = TRUE;

done:
    dbus_error_free (&error);

    if (reply) dbus_message_unref (reply);
    if (msg)   dbus_message_unref (msg);

    return success;
}

static void
process_queued_ops ()
{
    QueueItem *queued_volume = NULL;

    while ((queued_volume = g_queue_pop_head (volume_queue)) != NULL) {
        N_DEBUG (LOG_CAT "processing queued volume for role '%s', volume %d ",
            queued_volume->role, queued_volume->volume);
        add_entry (queued_volume->role, queued_volume->volume);

        g_free (queued_volume->role);
        g_slice_free (QueueItem, queued_volume);
    }
}

static gboolean
connect_to_pulseaudio ()
{
    const char *pulse_address = NULL;
    DBusError   error;

    if ((pulse_address = getenv ("PULSE_DBUS_SERVER")) == NULL)
        pulse_address = PULSE_DBUS_ADDRESS;

    dbus_error_init (&error);
    volume_bus = dbus_connection_open (pulse_address, &error);

    if (dbus_error_is_set (&error)) {
        N_WARNING (LOG_CAT "failed to open connection to pulseaudio: %s",
            error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    dbus_connection_setup_with_g_main (volume_bus, NULL);

    if (!dbus_connection_add_filter (volume_bus, filter_cb, NULL, NULL)) {
        N_WARNING (LOG_CAT "failed to add filter");
        return FALSE;
    }

    return TRUE;
}

static void
disconnect_from_pulseaudio ()
{
    if (volume_retry_id > 0) {
        g_source_remove (volume_retry_id);
        volume_retry_id = 0;
    }

    if (volume_bus) {
        dbus_connection_unref (volume_bus);
        volume_bus = NULL;
    }
}

int
volume_controller_initialize ()
{
    if ((volume_queue = g_queue_new ()) == NULL)
        return FALSE;

    if (!connect_to_pulseaudio ()) {
        N_DEBUG (LOG_CAT "failed to connect to Pulseaudio DBus, "
                         "reconnecting in %d seconds.", RETRY_TIMEOUT);

        volume_retry_id = g_timeout_add_seconds (RETRY_TIMEOUT,
            retry_timeout_cb, NULL);
    }

    return TRUE;
}

void
volume_controller_shutdown ()
{
    disconnect_from_pulseaudio ();

    if (volume_queue) {
        g_queue_free (volume_queue);
        volume_queue = NULL;
    }
}

int
volume_controller_update (const char *role, int volume)
{
    QueueItem *item = NULL;

    if (!role)
        return FALSE;

    if (!volume_bus) {
        N_DEBUG (LOG_CAT "volume controller not ready, queueing op.");

        item = g_slice_new0 (QueueItem);
        item->role   = g_strdup (role);
        item->volume = volume;
        g_queue_push_tail (volume_queue, item);
        return TRUE;
    }

    return add_entry (role, volume);
}
