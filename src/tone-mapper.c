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

#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "log.h"
#include "tone-mapper.h"

#define TONE_MANAGER_DBUS_NAME          "com.nokia.ToneManager1"
#define TONE_MANAGER_DBUS_PATH          "/com/nokia/ToneManager1"
#define TONE_MANAGER_DBUS_IFACE         "com.nokia.ToneManager1"

#define TONE_MANAGER_SIGNAL_COMPLETED   "Completed"
#define TONE_MANAGER_SIGNAL_REMOVED     "Removed"
#define TONE_MANAGER_METHOD_GETALL      "GetAll"
#define TONE_MANAGER_METHOD_GETTONEPATH "GetTonePath"

static const char *signal_matches[] = {
    "type=signal,interface=" TONE_MANAGER_DBUS_IFACE ",member=" TONE_MANAGER_SIGNAL_COMPLETED,
    "type=signal,interface=" TONE_MANAGER_DBUS_IFACE ",member=" TONE_MANAGER_SIGNAL_REMOVED,
    NULL
};

static DBusHandlerResult
filter_cb (DBusConnection *connection, DBusMessage *msg, void *userdata)
{
    Context    *context  = (Context*) userdata;
    const char *key      = NULL;
    const char *value    = NULL;
    gchar      *mapped_tone_path = NULL;

    if (dbus_message_is_signal (msg, TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_SIGNAL_COMPLETED)) {
        if (dbus_message_get_args (msg, NULL, DBUS_TYPE_STRING, &key, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
            mapped_tone_path = g_build_filename (context->mapped_tone_path, value, NULL);
            g_hash_table_replace (context->mapped_tones, g_strdup (key), mapped_tone_path);
            LOG_DEBUG ("%s >> new mapped tone: %s = %s\n", __FUNCTION__, key, mapped_tone_path);
        }
     }

    else if (dbus_message_is_signal (msg, TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_SIGNAL_REMOVED)) {
        if (dbus_message_get_args (msg, NULL, DBUS_TYPE_STRING, &key, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
            g_hash_table_remove (context->mapped_tones, key);
            LOG_DEBUG ("%s >> removed mapped tone: %s\n", __FUNCTION__, key);
        }
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
add_signal_matches (Context *context)
{
    const char **p = NULL;
    for (p = signal_matches; *p; p++)
        dbus_bus_add_match (context->session_bus, *p, NULL);

    dbus_connection_add_filter (context->session_bus, filter_cb, context, NULL);
}

static void
remove_signal_matches (Context *context)
{
    const char **p = NULL;
    for (p = signal_matches; *p; p++)
        dbus_bus_remove_match (context->session_bus, *p, NULL);

    dbus_connection_remove_filter (context->session_bus, filter_cb, context);
}

static const char*
get_string (DBusMessageIter *iter)
{
    const char *str = NULL;

    if (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_STRING) {
        dbus_message_iter_get_basic (iter, &str);
        dbus_message_iter_next (iter);
        return str;
    }

    return NULL;
}

static void
get_all_reply_cb (DBusPendingCall *pending, void *userdata)
{
    Context     *context       = (Context*) userdata;
    DBusMessage *msg           = NULL;
    const char  *key           = NULL;
    const char  *value         = NULL;
    gchar       *tone_filename = NULL;

    DBusMessageIter iter, array, dict;

    if ((msg = dbus_pending_call_steal_reply (pending)) == NULL)
        goto done;

    dbus_message_iter_init (msg, &iter);
    if (dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_ARRAY) {
        dbus_message_iter_recurse (&iter, &array);

        while (dbus_message_iter_get_arg_type (&array) == DBUS_TYPE_DICT_ENTRY) {
            dbus_message_iter_recurse (&array, &dict);

            key   = get_string (&dict);
            value = get_string (&dict);

            if (key && value) {
                tone_filename = g_build_filename (context->mapped_tone_path, value, NULL);
                g_hash_table_replace (context->mapped_tones, g_strdup (key), tone_filename);
                LOG_DEBUG ("%s >> mapped file %s => %s\n", __FUNCTION__, key, tone_filename);
            }

            dbus_message_iter_next (&array);
        }
    }

done:
    if (msg) {
        dbus_message_unref (msg);
        msg = NULL;
    }

    dbus_pending_call_unref (pending);
}

static gboolean
query_tones (Context *context)
{
    DBusMessage     *msg     = NULL;
    DBusPendingCall *pending = NULL;

    msg = dbus_message_new_method_call (TONE_MANAGER_DBUS_NAME, TONE_MANAGER_DBUS_PATH,
        TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_METHOD_GETALL);

    if (msg == NULL) {
        LOG_WARNING ("%s >> failed to create msg!", __FUNCTION__);
        return FALSE;
    }

    dbus_connection_send_with_reply (context->session_bus, msg, &pending, -1);
    dbus_message_unref (msg);

    if (pending == NULL) {
        LOG_WARNING ("%s >> failed to get pending!", __FUNCTION__);
        return FALSE;
    }

    dbus_pending_call_set_notify (pending, get_all_reply_cb, context, NULL);
    return TRUE;
}

static gboolean
query_tone_path (Context *context)
{
    DBusMessage *msg       = NULL;
    DBusMessage *reply     = NULL;
    const char  *mapped_tone_path = NULL;
    DBusError    error;

    msg = dbus_message_new_method_call (TONE_MANAGER_DBUS_NAME, TONE_MANAGER_DBUS_PATH,
        TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_METHOD_GETTONEPATH);

    if (msg == NULL) {
        LOG_WARNING ("%s >> failed to create msg!", __FUNCTION__);
        return FALSE;
    }

    dbus_error_init (&error);
    reply = dbus_connection_send_with_reply_and_block (context->session_bus, msg, -1, &error);

    if (reply) {
        if (dbus_message_get_args (reply, NULL, DBUS_TYPE_STRING, &mapped_tone_path, DBUS_TYPE_INVALID)) {
            LOG_DEBUG ("%s >> got tone path: %s", __FUNCTION__, mapped_tone_path);
            context->mapped_tone_path = g_strdup (mapped_tone_path);
        }
    }

    if (dbus_error_is_set (&error)) {
        LOG_ERROR ("%s >> query tone path failed: %s", __FUNCTION__, error.message);
        dbus_error_free (&error);
    }

    dbus_message_unref (msg);
    dbus_message_unref (reply);

    return TRUE;
}

int
tone_mapper_create (Context *context)
{
    gboolean success = FALSE;

    if ((context->mapped_tones = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free)) == NULL)
        return FALSE;

    success = tone_mapper_reconnect (context);
    (void) success;

    return TRUE;
}

int
tone_mapper_reconnect (Context *context)
{
    int success = FALSE;

    if (!context->session_bus) {
        LOG_DEBUG ("%s >> failed to connect to session bus.", __FUNCTION__);
        return FALSE;
    }

    success = query_tone_path (context);
    success = query_tones     (context);

    add_signal_matches (context);

    (void) success;
    return TRUE;
}

void
tone_mapper_destroy (Context *context)
{
    if (!context->session_bus)
        return;

    remove_signal_matches (context);

    if (context->mapped_tones) {
        g_hash_table_destroy (context->mapped_tones);
        context->mapped_tones = NULL;
    }

    g_free (context->mapped_tone_path);
}

const char*
tone_mapper_get_tone (Context *context, const char *orig)
{
    if (context == NULL || orig == NULL)
        return NULL;

    if (context->mapped_tones == NULL)
        return NULL;

    return (const char*) g_hash_table_lookup (context->mapped_tones, orig);
}
