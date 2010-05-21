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
#include <mce/dbus-names.h>

#include "log.h"
#include "led.h"

static gboolean
call_dbus_method (DBusConnection *bus, DBusMessage *msg)
{
    if (!dbus_connection_send (bus, msg, 0)) {
        LOG_WARNING ("Failed to send DBus message %s to interface %s", dbus_message_get_member (msg), dbus_message_get_interface (msg));
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
