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

#define TONEGEN_NAME   "com.Nokia.Telephony.Tones"
#define TONEGEN_PATH   "/com/Nokia/Telephony/Tones"
#define TONEGEN_IFACE  "com.Nokia.Telephony.Tones"
#define TONEGEN_START  "StartEventTone"
#define TONEGEN_STOP   "StopTone"

#define TONEGEN_VOLUME -5

#include <dbus/dbus.h>

#include "log.h"
#include "tone-generator.h"

static gboolean
call_dbus_method (DBusConnection *bus, DBusMessage *msg)
{
    if (!dbus_connection_send (bus, msg, 0)) {
        NGF_LOG_WARNING ("Failed to send DBus message %s to interface %s", dbus_message_get_member (msg), dbus_message_get_interface (msg));
        return FALSE;
    }

    return TRUE;
}

static gboolean
tone_generator_toggle (DBusConnection *bus, guint pattern, gboolean activate)
{
    DBusMessage    *msg     = NULL;
    dbus_int32_t    volume  = TONEGEN_VOLUME;
    dbus_uint32_t   length  = 0;
    gboolean        ret     = FALSE;

    if (bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (TONEGEN_NAME,
                                        TONEGEN_PATH,
                                        TONEGEN_IFACE,
                                        activate ? TONEGEN_START : TONEGEN_STOP);

    if (msg == NULL)
        return FALSE;

    if (activate) {
        ret = dbus_message_append_args (msg,
            DBUS_TYPE_UINT32, &pattern,
            DBUS_TYPE_INT32, &volume,
            DBUS_TYPE_UINT32, &length,
            DBUS_TYPE_INVALID);

        if (!ret) {
            dbus_message_unref (msg);
            return FALSE;
        }
    }

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);
    return ret;
}

gboolean
tone_generator_start (DBusConnection *system_bus, guint pattern)
{
    return tone_generator_toggle (system_bus, pattern, TRUE);
}

gboolean
tone_generator_stop (DBusConnection *system_bus, guint pattern)
{
    return tone_generator_toggle (system_bus, pattern, FALSE);
}
