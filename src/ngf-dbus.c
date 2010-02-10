/*
 * ngfd - Non-graphical feedback daemon
 * This file is part of ngfd.
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

#include "ngf-dbus.h"

struct _NgfDBus
{
    DBusConnection      *connection;
    NgfDBusPlayFunction play_function;
    NgfDBusStopFunction stop_function;
    gpointer            userdata;
};

static gboolean
_msg_parse_variant (DBusMessageIter *iter, NgfValue **value)
{
    DBusMessageIter variant;

    const char *str_value = NULL;
    dbus_uint32_t uint_value;
    dbus_int32_t int_value;
    dbus_bool_t boolean_value;

    if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_VARIANT)
        return FALSE;

    dbus_message_iter_recurse (iter, &variant);

    *value = ngf_value_new ();

    switch (dbus_message_iter_get_arg_type (&variant)) {
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic (&variant, &str_value);
            ngf_value_set_string (*value, str_value);
            return TRUE;

        case DBUS_TYPE_UINT32:
            dbus_message_iter_get_basic (&variant, &uint_value);
            ngf_value_set_uint (*value, uint_value);
            return TRUE;

        case DBUS_TYPE_INT32:
            dbus_message_iter_get_basic (&variant, &int_value);
            ngf_value_set_int (*value, int_value);
            return TRUE;

        case DBUS_TYPE_BOOLEAN:
            dbus_message_iter_get_basic (&variant, &boolean_value);
            ngf_value_set_boolean (*value, boolean_value ? TRUE : FALSE);
            return TRUE;

        default:
            ngf_value_free (*value);
            *value = NULL;
            break;
    }

    return FALSE;
}

static gboolean
_msg_parse_dict (DBusMessageIter *iter, const char **key, NgfValue **value)
{
    DBusMessageIter dict;

    /* Recurse to the dict entry */

    if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_DICT_ENTRY)
        return FALSE;

    dbus_message_iter_recurse (iter, &dict);

    /* Get the key for the dict entry */

    if (dbus_message_iter_get_arg_type (&dict) != DBUS_TYPE_STRING)
        return FALSE;

    dbus_message_iter_get_basic (&dict, key);
    dbus_message_iter_next (&dict);

    /* Parse the variant contents */
    if (!_msg_parse_variant (&dict, value))
        return FALSE;

    return TRUE;
}

static void
_value_free (gpointer data)
{
    ngf_value_free ((NgfValue*) data);
}

static gboolean
_msg_get_properties (DBusMessageIter *iter, GHashTable **properties)
{
    DBusMessageIter array;
    GHashTable *p = NULL;

    const char *key = NULL;
    NgfValue *value = NULL;

    if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_ARRAY)
        return FALSE;

    p = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _value_free);

    dbus_message_iter_recurse (iter, &array);

    while (dbus_message_iter_get_arg_type (&array) != DBUS_TYPE_INVALID) {
        if (_msg_parse_dict (&array, &key, &value)) {
            g_hash_table_insert (p, g_strdup (key), value);
        }

        dbus_message_iter_next (&array);
    }

    *properties = p;

    return TRUE;
}

static DBusHandlerResult
_handle_play (DBusConnection *connection,
              DBusMessage *msg,
              void *userdata)
{
    NgfDBus *self = (NgfDBus*) userdata;

    DBusMessage *reply = NULL;
    DBusMessageIter iter;
    const char *event = NULL;
    GHashTable *properties = NULL;
    guint id = 0;

    dbus_message_iter_init (msg, &iter);
    if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
        goto invalid;

    dbus_message_iter_get_basic (&iter, &event);
    dbus_message_iter_next (&iter);

    if (!_msg_get_properties (&iter, &properties))
        goto invalid;

    /* Call the play function handler */

    if (self->play_function)
        id = self->play_function (self, event, properties, self->userdata);

    reply = dbus_message_new_method_return (msg);
    if (reply) {
        dbus_message_append_args (reply, DBUS_TYPE_UINT32, &id, DBUS_TYPE_INVALID);
        dbus_connection_send (connection, reply, NULL);
        dbus_message_unref (reply);
    }

invalid:
    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
_handle_stop (DBusConnection *connection,
              DBusMessage *msg,
              void *userdata)
{
    NgfDBus *self = (NgfDBus*) userdata;

    DBusMessage *reply = NULL;
    dbus_uint32_t id = 0;

    if (dbus_message_get_args (msg, NULL, DBUS_TYPE_UINT32, &id, DBUS_TYPE_INVALID)) {
        if (self->stop_function)
            self->stop_function (self, (guint) id, self->userdata);
    }

    reply = dbus_message_new_method_return (msg);
    if (reply) {
        dbus_connection_send (connection, reply, NULL);
        dbus_message_unref (reply);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
_message_function (DBusConnection *connection,
                   DBusMessage *msg,
                   void *userdata)
{
    const char *member = dbus_message_get_member (msg);
    const char *sender = dbus_message_get_sender (msg);

    if (!dbus_message_has_interface (msg, NGF_DBUS_IFACE))
        return DBUS_HANDLER_RESULT_HANDLED;

    if (g_str_equal (member, NGF_DBUS_BACKEND_PLAY))
        return _handle_play (connection, msg, userdata);

    else if (g_str_equal (member, NGF_DBUS_BACKEND_STOP))
        return _handle_stop (connection, msg, userdata);

    return DBUS_HANDLER_RESULT_HANDLED;
}

gboolean
_dbus_initialize (NgfDBus *self, const char *name, const char *path)
{
    static struct DBusObjectPathVTable method = {
        .message_function = _message_function
    };

    DBusError error;
    int ret;

    dbus_error_init (&error);
    self->connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
    if (self->connection == NULL) {
        g_warning ("%s: Failed to get DBus: %s", __FUNCTION__, error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    dbus_connection_setup_with_g_main (self->connection, NULL);

    ret = dbus_bus_request_name (self->connection, name, DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        if (dbus_error_is_set (&error)) {
            g_warning ("%s: Failed to get unique name: %s", __FUNCTION__, error.message);
            dbus_error_free (&error);
        }
        return FALSE;
    }

    if (!dbus_connection_register_object_path (self->connection, path, &method, self))
        return FALSE;

    return TRUE;
}

NgfDBus*
ngf_dbus_create (NgfDBusPlayFunction play, NgfDBusStopFunction stop, gpointer userdata)
{
    NgfDBus *self = NULL;

    if ((self = g_new0 (NgfDBus, 1)) == NULL)
        return NULL;

    if (!_dbus_initialize (self, NGF_DBUS_BACKEND_NAME, NGF_DBUS_PATH)) {
        ngf_dbus_destroy (self);
        return NULL;
    }

    self->play_function = play;
    self->stop_function = stop;
    self->userdata = userdata;

    return self;
}

void
ngf_dbus_destroy (NgfDBus *self)
{
    if (self == NULL)
        return;

    if (self->connection) {
        dbus_connection_unref (self->connection);
        self->connection = NULL;
    }

    g_free (self);
}

void
ngf_dbus_send_status (NgfDBus *self, guint id, guint status)
{
    if (self == NULL)
        return;

    DBusMessage *msg = NULL;

    if ((msg = dbus_message_new_method_call (NGF_DBUS_PROXY_NAME, NGF_DBUS_PATH, NGF_DBUS_IFACE, NGF_DBUS_STATUS)) == NULL)
        return;

    dbus_message_append_args (msg,
        DBUS_TYPE_UINT32, &id,
        DBUS_TYPE_UINT32, &status,
        DBUS_TYPE_INVALID);

    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);
}
