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
#include <mce/dbus-names.h>

#include "log.h"
#include "led.h"

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
toggle_pattern (DBusConnection *bus, const char *pattern, gboolean activate)
{
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (bus == NULL || pattern == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF,
        activate ? MCE_ACTIVATE_LED_PATTERN : MCE_DEACTIVATE_LED_PATTERN);

    if (msg == NULL)
        return FALSE;

    if (!dbus_message_append_args (msg, DBUS_TYPE_STRING, &pattern, DBUS_TYPE_INVALID)) {
        dbus_message_unref (msg);
        return FALSE;
    }

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);

    if (ret)
        N_DEBUG ("%s >> led pattern %s %s.", __FUNCTION__, pattern, activate ? "activated" : "deactivated");

    return ret;
}

gboolean
led_activate_pattern (DBusConnection *system_bus, const char *pattern)
{
    return toggle_pattern (system_bus, pattern, TRUE);
}

gboolean
led_deactivate_pattern (DBusConnection *system_bus, const char *pattern)
{
    return toggle_pattern (system_bus, pattern, FALSE);
}
