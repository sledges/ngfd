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

#define DBUS_TONE_GENERATOR_NAME   "com.Nokia.Telephony.Tones"
#define DBUS_TONE_GENERATOR_PATH   "/com/Nokia/Telephony/Tones"
#define DBUS_TONE_GENERATOR_IFACE  "com.Nokia.Telephony.Tones"
#define DBUS_TONE_GENERATOR_START  "StartEventTone"
#define DBUS_TONE_GENERATOR_STOP   "StopTone"

#define TONE_GENERATOR_VOLUME -5

#include <dbus/dbus.h>
#include "tone-generator.h"

struct _ToneGenerator
{
    DBusConnection  *connection;
};

ToneGenerator*
tone_generator_create ()
{
    ToneGenerator *self = NULL;
    DBusError error;

    if ((self = g_new0 (ToneGenerator, 1)) == NULL)
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
tone_generator_destroy (ToneGenerator *self)
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
_tone_generator_toggle (ToneGenerator *self, guint pattern, guint play)
{
    DBusMessage *msg = NULL;

    dbus_int32_t    volume  = TONE_GENERATOR_VOLUME;
    dbus_uint32_t   length  = 0;
    gint            success = 0;

    if (self->connection == NULL)
        return;

    msg = dbus_message_new_method_call (DBUS_TONE_GENERATOR_NAME,
                                        DBUS_TONE_GENERATOR_PATH,
                                        DBUS_TONE_GENERATOR_IFACE,
                                        play ? DBUS_TONE_GENERATOR_START : DBUS_TONE_GENERATOR_STOP);

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

guint
tone_generator_start (ToneGenerator *self, guint pattern)
{
    _tone_generator_toggle (self, pattern, TRUE);
    return 1;
}

void
tone_generator_stop (ToneGenerator *self, guint id)
{
    if (id > 0)
        _tone_generator_toggle (self, 0, FALSE);
}
