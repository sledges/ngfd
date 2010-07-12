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
#include <mce/mode-names.h>

#include "log.h"
#include "backlight.h"

#define DISPLAY_BLANK_TIMEOUT 50

static guint prevent_display_blank_id = 0;

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
prevent_display_blank (gpointer userdata)
{
    DBusConnection *bus = (DBusConnection*) userdata;
    DBusMessage    *msg = NULL;
    gboolean        ret = FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_PREVENT_BLANK_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (bus, msg);
    dbus_message_unref (msg);

    return ret;
}

gboolean
backlight_unlock_tklock (DBusConnection *system_bus)
{
    DBusMessage *msg     = NULL;
    gboolean     ret     = FALSE;
    const gchar *command = MCE_TK_UNLOCKED;

    if (system_bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_TKLOCK_MODE_CHANGE_REQ);
    if (msg == NULL)
        return FALSE;

    if (!dbus_message_append_args (msg, DBUS_TYPE_STRING, &command, DBUS_TYPE_INVALID)) {
        dbus_message_unref (msg);
        return FALSE;
    }

    ret = call_dbus_method (system_bus, msg);
    dbus_message_unref (msg);
    return ret;
}

gboolean
backlight_display_on (DBusConnection *system_bus)
{
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (system_bus == NULL)
        return FALSE;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DISPLAY_ON_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (system_bus, msg);
    dbus_message_unref (msg);

    return ret;
}

gboolean
backlight_prevent_blank (DBusConnection *system_bus)
{
    if (system_bus == NULL)
        return FALSE;

    if (prevent_display_blank_id > 0)
        g_source_remove (prevent_display_blank_id);

    prevent_display_blank_id = g_timeout_add_seconds (DISPLAY_BLANK_TIMEOUT, prevent_display_blank, system_bus);

    return (gboolean) prevent_display_blank_id;
}

gboolean
backlight_cancel_prevent_blank (DBusConnection *system_bus)
{
    DBusMessage *msg = NULL;
    gboolean     ret = FALSE;

    if (system_bus == NULL)
        return FALSE;

    if (prevent_display_blank_id > 0) {
        g_source_remove (prevent_display_blank_id);
        prevent_display_blank_id = 0;
    }

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_CANCEL_PREVENT_BLANK_REQ);
    if (msg == NULL)
        return FALSE;

    ret = call_dbus_method (system_bus, msg);
    dbus_message_unref (msg);

    return ret;
}
