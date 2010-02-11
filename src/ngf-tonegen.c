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

#define NGF_DBUS_TONEGEN_NAME   "com.Nokia.Telephony.Tones"
#define NGF_DBUS_TONEGEN_PATH   "/com/Nokia/Telephony/Tones"
#define NGF_DBUS_TONEGEN_IFACE  "com.Nokia.Telephony.Tones"
#define NGF_DBUS_TONEGEN_START  "StartEventTone"
#define NGF_DBUS_TONEGEN_STOP   "StopTone"

#define NGF_TONEGEN_VOLUME -5

#include <dbus/dbus.h>
#include "ngf-tonegen.h"

struct _NgfTonegen
{
    DBusConnection  *connection;
};

NgfTonegen*
ngf_tonegen_create ()
{
    NgfTonegen *self = NULL;
    DBusError error;

    if ((self = g_new0 (NgfTonegen, 1)) == NULL)
        return NULL;

    dbus_error_init (&error);
    if ((self->connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error)) == NULL) {
        if (dbus_error_is_set (&error))
            dbus_error_free (&error);

        g_free (self);
        return NULL;
    }

    return self;
}

void
ngf_tonegen_destroy (NgfTonegen *self)
{
    if (self == NULL)
        return;

    if (self->connection) {
        dbus_connection_unref (self->connection);
        self->connection = NULL;
    }

    g_free (self);
}

static void
_tonegen_toggle (NgfTonegen *self, guint pattern, guint play)
{
    DBusMessage *msg = NULL;

    dbus_int32_t    volume  = NGF_TONEGEN_VOLUME;
    dbus_uint32_t   length  = 0;
    gint            success = 0;

    if (self->connection == NULL)
        return;

    msg = dbus_message_new_method_call (NGF_DBUS_TONEGEN_NAME,
                                        NGF_DBUS_TONEGEN_PATH,
                                        NGF_DBUS_TONEGEN_IFACE,
                                        play ? NGF_DBUS_TONEGEN_START : NGF_DBUS_TONEGEN_STOP);

    if (msg == NULL)
        return;

    if (play) {
        success = dbus_message_append_args (msg,
            DBUS_TYPE_UINT32, &pattern,
            DBUS_TYPE_INT32, &volume,
            DBUS_TYPE_UINT32, &length,
            DBUS_TYPE_INVALID);

        if (!success) {
            dbus_message_unref (msg);
            return;
        }
    }

    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);
}

void
ngf_tonegen_start (NgfTonegen *self, guint pattern)
{
    _tonegen_toggle (self, pattern, TRUE);
}

void
ngf_tonegen_stop (NgfTonegen *self, guint pattern)
{
    _tonegen_toggle (self, pattern, FALSE);
}
