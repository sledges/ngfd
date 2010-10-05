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
#include "proplist.h"
#include "state.h"
#include "dbus-if.h"

#define NGF_DBUS_PROXY_NAME  "com.nokia.NonGraphicFeedback1"
#define NGF_DBUS_NAME        "com.nokia.NonGraphicFeedback1.Backend"
#define NGF_DBUS_PATH        "/com/nokia/NonGraphicFeedback1"
#define NGF_DBUS_IFACE       "com.nokia.NonGraphicFeedback1"

#define NGF_DBUS_UPDATE      "Update"
#define NGF_DBUS_STATUS      "Status"

#define NGF_DBUS_METHOD_PLAY  "Play"
#define NGF_DBUS_METHOD_STOP  "Stop"
#define NGF_DBUS_METHOD_PAUSE "Pause"

static gboolean
_msg_parse_variant (DBusMessageIter *iter, NValue **value)
{
    DBusMessageIter variant;

    const char *str_value = NULL;
    dbus_uint32_t uint_value;
    dbus_int32_t int_value;
    dbus_bool_t boolean_value;

    if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_VARIANT)
        return FALSE;

    dbus_message_iter_recurse (iter, &variant);

    *value = n_value_new ();

    switch (dbus_message_iter_get_arg_type (&variant)) {
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic (&variant, &str_value);
            n_value_set_string (*value, str_value);
            return TRUE;

        case DBUS_TYPE_UINT32:
            dbus_message_iter_get_basic (&variant, &uint_value);
            n_value_set_uint (*value, uint_value);
            return TRUE;

        case DBUS_TYPE_INT32:
            dbus_message_iter_get_basic (&variant, &int_value);
            n_value_set_int (*value, int_value);
            return TRUE;

        case DBUS_TYPE_BOOLEAN:
            dbus_message_iter_get_basic (&variant, &boolean_value);
            n_value_set_bool (*value, boolean_value ? TRUE : FALSE);
            return TRUE;

        default:
            n_value_free (*value);
            *value = NULL;
            break;
    }

    return FALSE;
}

static gboolean
_msg_parse_dict (DBusMessageIter *iter, const char **key, NValue **value)
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

static gboolean
_msg_get_properties (DBusMessageIter *iter, NProplist **proplist)
{
    NProplist  *p        = NULL;
    const char *key      = NULL;
    NValue     *value    = NULL;
    DBusMessageIter array;

    if (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_ARRAY)
        return FALSE;

    p = n_proplist_new ();

    dbus_message_iter_recurse (iter, &array);

    while (dbus_message_iter_get_arg_type (&array) != DBUS_TYPE_INVALID) {
        if (_msg_parse_dict (&array, &key, &value) && key && value) {
            if (key)
                n_proplist_set (p, key, value);
        }

        dbus_message_iter_next (&array);
    }

    *proplist = p;

    return TRUE;
}

static DBusHandlerResult
_handle_play (DBusConnection *connection,
              DBusMessage    *msg,
              void           *userdata)
{
    Context *context = (Context*) userdata;

    DBusMessage *reply      = NULL;
    const char  *event      = NULL;
    NProplist   *proplist   = NULL;
    guint        id         = 0;

    DBusMessageIter iter;

    dbus_message_iter_init (msg, &iter);
    if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_STRING)
        goto invalid;

    dbus_message_iter_get_basic (&iter, &event);
    dbus_message_iter_next (&iter);

    if (!_msg_get_properties (&iter, &proplist))
        goto invalid;

    /* call the play handler */
    id = play_handler (context, event, proplist);
    n_proplist_free (proplist);

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
    Context *context = (Context*) userdata;

    DBusMessage   *reply = NULL;
    dbus_uint32_t  id    = 0;

    if (dbus_message_get_args (msg, NULL, DBUS_TYPE_UINT32, &id, DBUS_TYPE_INVALID)) {
        /* call the stop handler */
        stop_handler (context, id);
    }

    reply = dbus_message_new_method_return (msg);
    if (reply) {
        dbus_connection_send (connection, reply, NULL);
        dbus_message_unref (reply);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
_handle_pause (DBusConnection *connection,
               DBusMessage *msg,
               void *userdata)
{
    Context *context = (Context*) userdata;

    DBusMessage   *reply = NULL;
    dbus_uint32_t  id    = 0;
    dbus_bool_t    pause = 0;

    if (dbus_message_get_args (msg, NULL,
                               DBUS_TYPE_UINT32, &id,
                               DBUS_TYPE_BOOLEAN, &pause,
                               DBUS_TYPE_INVALID)) {
        /* call the pause handler */
        pause_handler (context, id, pause);
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

    if (member == NULL)
        return DBUS_HANDLER_RESULT_HANDLED;

    if (!dbus_message_has_interface (msg, NGF_DBUS_IFACE))
        return DBUS_HANDLER_RESULT_HANDLED;

    if (g_str_equal (member, NGF_DBUS_METHOD_PLAY))
        return _handle_play (connection, msg, userdata);

    else if (g_str_equal (member, NGF_DBUS_METHOD_STOP))
        return _handle_stop (connection, msg, userdata);

    else if (g_str_equal (member, NGF_DBUS_METHOD_PAUSE))
        return _handle_pause (connection, msg, userdata);

    return DBUS_HANDLER_RESULT_HANDLED;
}

gboolean
_dbus_initialize (Context *context, const char *name, const char *path)
{
    static struct DBusObjectPathVTable method = {
        .message_function = _message_function
    };

    DBusError error;
    int       ret;

    dbus_error_init (&error);
    ret = dbus_bus_request_name (context->system_bus, name, DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        if (dbus_error_is_set (&error)) {
            NGF_LOG_WARNING ("Failed to get unique name: %s", error.message);
            dbus_error_free (&error);
        }

        return FALSE;
    }

    if (!dbus_connection_register_object_path (context->system_bus, path, &method, context))
        return FALSE;

    return TRUE;
}

int
dbus_if_create (Context *context)
{
    if (!_dbus_initialize (context, NGF_DBUS_NAME, NGF_DBUS_PATH)) {
        dbus_if_destroy (context);
        return FALSE;
    }

    return TRUE;
}

void
dbus_if_destroy (Context *context)
{
    (void) context;
}

void
dbus_if_send_status (Context *context, guint id, guint status)
{
    DBusMessage *msg = NULL;

    if ((msg = dbus_message_new_method_call (NGF_DBUS_PROXY_NAME, NGF_DBUS_PATH, NGF_DBUS_IFACE, NGF_DBUS_STATUS)) == NULL)
        return;

    dbus_message_append_args (msg,
        DBUS_TYPE_UINT32, &id,
        DBUS_TYPE_UINT32, &status,
        DBUS_TYPE_INVALID);

    dbus_connection_send (context->system_bus, msg, NULL);
    dbus_message_unref (msg);
}
