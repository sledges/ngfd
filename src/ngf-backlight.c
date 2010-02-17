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
#include "ngf-backlight.h"

struct _NgfBacklight
{
    DBusConnection *connection;
    GHashTable     *patterns;
};

NgfBacklight*
ngf_backlight_create ()
{
    NgfBacklight *self = NULL;
    DBusError error;

    if ((self = g_new0 (NgfBacklight, 1)) == NULL)
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
ngf_backlight_destroy (NgfBacklight *self)
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
_toggle_state (NgfBacklight *self, gint state)
{
    DBusMessage *msg = NULL;
    const char *command = NULL;

    if (self->connection == NULL)
        return;

    switch (state) {
        case 0:
            command = MCE_DISPLAY_OFF_REQ;
            break;

        case 1:
            command = MCE_DISPLAY_DIM_REQ;
            break;

        case 2:
            command = MCE_DISPLAY_ON_REQ;
            break;

        default:
            return;
    }

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, command);
    if (msg == NULL)
        return;

    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);
}

void
ngf_backlight_toggle (NgfBacklight *self, gint state)
{
    _toggle_state (self, state);
}

