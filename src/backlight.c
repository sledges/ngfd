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
