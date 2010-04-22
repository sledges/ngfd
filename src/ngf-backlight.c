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

#include "ngf-controller.h"
#include "ngf-backlight.h"

struct _NgfBacklight
{
    DBusConnection *connection;
    guint blank_timeout;
};

NgfBacklight*
ngf_backlight_create ()
{
    NgfBacklight *self = NULL;
    DBusError error;

    if ((self = g_new0 (NgfBacklight, 1)) == NULL)
        goto failed;

    dbus_error_init (&error);
    if ((self->connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error)) == NULL) {
        if (dbus_error_is_set (&error))
            dbus_error_free (&error);

        goto failed;
    }
    
    self->blank_timeout = 0;

    return self;

failed:
    ngf_backlight_destroy (self);
    return NULL;
}

void
ngf_backlight_destroy (NgfBacklight *self)
{
    if (self == NULL)
        return;

    if (self->connection) {
        dbus_connection_unref (self->connection);
        self->connection = NULL;
    }
    
    if (self->blank_timeout) {
        g_source_remove (self->blank_timeout);
        self->blank_timeout = 0;
    }

    g_free (self);
}

gboolean
_ngf_prevent_display_blank (gpointer data)
{
    NgfBacklight *self = (NgfBacklight*) data;
    DBusMessage *msg = NULL;
    
    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_PREVENT_BLANK_REQ);
    if (msg == NULL)
        return FALSE;

    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);
    
    return TRUE;
}

gboolean
ngf_backlight_start (NgfBacklight *self, gboolean unlock)
{
    DBusMessage *msg = NULL;
    
    if (self == NULL)
        return FALSE;
    
    if (unlock) {
        msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_TKLOCK_MODE_CHANGE_REQ);
        if (msg == NULL)
            return FALSE;
        
        if (!dbus_message_append_args (msg, DBUS_TYPE_STRING, MCE_TK_UNLOCKED, DBUS_TYPE_INVALID)) {
            dbus_message_unref (msg);
            return FALSE;
        }
        
        dbus_connection_send (self->connection, msg, NULL);
        dbus_message_unref (msg);
        msg = NULL;
    }
        
    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DISPLAY_ON_REQ);
    if (msg == NULL)
        return FALSE;
    
    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);    

    self->blank_timeout = g_timeout_add_seconds (50, _ngf_prevent_display_blank, self);
    
    _ngf_prevent_display_blank (self);

    return TRUE;
}

void
ngf_backlight_stop (NgfBacklight *self)
{
    DBusMessage *msg = NULL;
    
    if (self == NULL)
        return;

    if (self->blank_timeout) {
        g_source_remove (self->blank_timeout);
        self->blank_timeout = 0;
    }
    
    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_CANCEL_PREVENT_BLANK_REQ);
    if (msg == NULL)
        return;

    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);
}
