/*
 * ngfd - Non-graphical feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation. All rights reserved.
 *
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating,
 * any or all of this material requires the prior written consent of
 * Nokia Corporation. This material also contains confidential
 * information which may not be disclosed to others without the prior
 * written consent of Nokia.
 */

#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "ngf-log.h"
#include "ngf-tone-mapper.h"

#define TONE_MANAGER_DBUS_NAME          "com.nokia.ToneManager1"
#define TONE_MANAGER_DBUS_PATH          "/com/nokia/ToneManager1"
#define TONE_MANAGER_DBUS_IFACE         "com.nokia.ToneManager1"
#define TONE_MANAGER_SIGNAL_COMPLETED   "Completed"
#define TONE_MANAGER_SIGNAL_REMOVED     "Removed"
#define TONE_MANAGER_METHOD_GETALL      "GetAll"
#define TONE_MANAGER_METHOD_GETTONEPATH "GetTonePath"

struct _NgfToneMapper
{
    DBusConnection  *connection;
    GHashTable      *tones;
    gchar           *tone_path;
};

static const char *signal_matches[] = {
    "type=signal,interface=" TONE_MANAGER_DBUS_IFACE ",member=" TONE_MANAGER_SIGNAL_COMPLETED,
    "type=signal,interface=" TONE_MANAGER_DBUS_IFACE ",member=" TONE_MANAGER_SIGNAL_REMOVED,
    NULL
};

static DBusHandlerResult
_filter_cb (DBusConnection *c, DBusMessage *msg, void *userdata)
{
    NgfToneMapper *self = (NgfToneMapper*) userdata;
    const char *key = NULL, *value = NULL;
    gchar *tone_path = NULL;

    if (dbus_message_is_signal (msg, TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_SIGNAL_COMPLETED)) {
        if (dbus_message_get_args (msg, NULL, DBUS_TYPE_STRING, &key, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
            tone_path = g_build_filename (self->tone_path, value, NULL);
            g_hash_table_replace (self->tones, g_strdup (key), tone_path);
            LOG_DEBUG ("New mapped tone: %s = %s\n", key, tone_path);
        }
     }

    else if (dbus_message_is_signal (msg, TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_SIGNAL_REMOVED)) {
        if (dbus_message_get_args (msg, NULL, DBUS_TYPE_STRING, &key, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
            g_hash_table_remove (self->tones, key);
            LOG_DEBUG ("Removed mapped tone: %s\n", key);
        }
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
_add_signal_matches (NgfToneMapper *self)
{
    const char **p = NULL;
    for (p = signal_matches; *p; p++)
        dbus_bus_add_match (self->connection, *p, NULL);

    dbus_connection_add_filter (self->connection, _filter_cb, self, NULL);
}

static void
_remove_signal_matches (NgfToneMapper *self)
{
    const char **p = NULL;
    for (p = signal_matches; *p; p++)
        dbus_bus_remove_match (self->connection, *p, NULL);

    dbus_connection_remove_filter (self->connection, _filter_cb, self);
}

static const char*
_get_string (DBusMessageIter *iter)
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
_get_all_reply_cb (DBusPendingCall *pending, void *userdata)
{
    NgfToneMapper *self = (NgfToneMapper*) userdata;

    DBusMessage *msg = NULL;
    DBusMessageIter iter, array, dict;
    const char *key = NULL, *value = NULL;
    gchar *tone_filename = NULL;

    if ((msg = dbus_pending_call_steal_reply (pending)) == NULL)
        goto done;

    dbus_message_iter_init (msg, &iter);
    if (dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_ARRAY) {
        dbus_message_iter_recurse (&iter, &array);

        while (dbus_message_iter_get_arg_type (&array) == DBUS_TYPE_DICT_ENTRY) {
            dbus_message_iter_recurse (&array, &dict);

            key = _get_string (&dict);
            value = _get_string (&dict);

            if (key && value) {
                tone_filename = g_build_filename (self->tone_path, value, NULL);
                g_hash_table_replace (self->tones, g_strdup (key), tone_filename);
                LOG_DEBUG ("key = %s, value = %s\n", key, tone_filename);
            }

            dbus_message_iter_next (&array);
        }
    }

done:
    dbus_pending_call_unref (pending);
}

static gboolean
_query_tones (NgfToneMapper *self)
{
    DBusMessage *msg = NULL;
    DBusPendingCall *pending = NULL;

    msg = dbus_message_new_method_call (TONE_MANAGER_DBUS_NAME, TONE_MANAGER_DBUS_PATH,
        TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_METHOD_GETALL);

    if (msg == NULL) {
        LOG_ERROR ("Failed to create msg!");
        return FALSE;
    }

    dbus_connection_send_with_reply (self->connection, msg, &pending, -1);
    dbus_message_unref (msg);

    if (pending == NULL) {
        LOG_ERROR ("failed to get pending!");
        return FALSE;
    }

    dbus_pending_call_set_notify (pending, _get_all_reply_cb, self, NULL);
    return TRUE;
}

static gboolean
_query_tone_path (NgfToneMapper *self)
{
    DBusMessage *msg = NULL, *reply = NULL;
    const char *tone_path = NULL;
    DBusError error;

    msg = dbus_message_new_method_call (TONE_MANAGER_DBUS_NAME, TONE_MANAGER_DBUS_PATH,
        TONE_MANAGER_DBUS_IFACE, TONE_MANAGER_METHOD_GETTONEPATH);

    if (msg == NULL) {
        LOG_ERROR ("Failed to create msg!");
        return FALSE;
    }

    dbus_error_init (&error);
    reply = dbus_connection_send_with_reply_and_block (self->connection, msg, -1, &error);

    if (reply) {
        if (dbus_message_get_args (reply, NULL, DBUS_TYPE_STRING, &tone_path, DBUS_TYPE_INVALID)) {
            LOG_DEBUG ("Got tone path: %s", tone_path);
            self->tone_path = g_strdup (tone_path);
        }
    }

    if (dbus_error_is_set (&error)) {
        LOG_ERROR ("%s: %s", __FUNCTION__, error.message);
        dbus_error_free (&error);
    }

    dbus_message_unref (msg);
    dbus_message_unref (reply);

    return TRUE;
}

NgfToneMapper*
ngf_tone_mapper_create ()
{
    NgfToneMapper *self = NULL;
    DBusError error;

    if ((self = g_new0 (NgfToneMapper, 1)) == NULL) {
        goto failed;
    }

    dbus_error_init (&error);
    self->connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
    if (self->connection == NULL) {
        if (dbus_error_is_set (&error)) {
            LOG_ERROR ("Failed to get session bus: %s", error.message);
            dbus_error_free (&error);
        }

        goto failed;
    }

    dbus_connection_setup_with_g_main (self->connection, NULL);

    if ((self->tones = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free)) == NULL)
        goto failed;

    /* Do the initial query of tone path and mapped tones. */
    if (!_query_tone_path (self)) {
        LOG_ERROR ("Failed to query tone path!");
        goto failed;
    }

    if (!_query_tones (self)) {
        LOG_ERROR ("Failed to query tones!");
        goto failed;
    }

    _add_signal_matches (self);

    return self;

failed:
    ngf_tone_mapper_destroy (self);
    return NULL;
}

void
ngf_tone_mapper_destroy (NgfToneMapper *self)
{
    if (self == NULL)
        return;

    if (self->connection) {
        _remove_signal_matches (self);
        dbus_connection_unref (self->connection);
        self->connection = NULL;
    }

    if (self->tones) {
        g_hash_table_destroy (self->tones);
        self->tones = NULL;
    }

    g_free (self->tone_path);

    g_free (self);
}

const char*
ngf_tone_mapper_get_tone (NgfToneMapper *self, const char *uri)
{
    if (self == NULL || uri == NULL)
        return NULL;

    if (self->tones == NULL)
        return NULL;

    return (const char*) g_hash_table_lookup (self->tones, uri);
}
