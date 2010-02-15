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
#include "ngf-led.h"

typedef struct _Pattern
{
    guint   id;
    gchar   *pattern;
} Pattern;

struct _NgfLed
{
    DBusConnection *connection;
    guint           pattern_count;
    GList           *active_patterns;
};

NgfLed*
ngf_led_create ()
{
    NgfLed *self = NULL;
    DBusError error;

    if ((self = g_new0 (NgfLed, 1)) == NULL)
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
ngf_led_destroy (NgfLed *self)
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
_toggle_pattern (NgfLed *self, const char *pattern, gboolean activate)
{
    DBusMessage *msg = NULL;

    if (self->connection == NULL)
        return;

    msg = dbus_message_new_method_call (MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF,
        activate ? MCE_ACTIVATE_LED_PATTERN : MCE_DEACTIVATE_LED_PATTERN);

    if (msg == NULL)
        return;

    if (!dbus_message_append_args (msg, DBUS_TYPE_STRING, &pattern, DBUS_TYPE_INVALID)) {
        dbus_message_unref (msg);
        return;
    }

    dbus_connection_send (self->connection, msg, NULL);
    dbus_message_unref (msg);
}

guint
ngf_led_start (NgfLed *self, const char *pattern)
{
    Pattern *p = NULL;

    if (self == NULL || pattern == NULL)
        return 0;

    if ((p = g_new0 (Pattern, 1)) == NULL)
        return 0;

    p->id = ++self->pattern_count;
    p->pattern = g_strdup (pattern);

    self->active_patterns = g_list_append (self->active_patterns, p);
    _toggle_pattern (self, p->pattern, TRUE);

    return p->id;
}

void
ngf_led_stop (NgfLed *self, guint id)
{
    GList *iter = NULL;
    Pattern *p = NULL;

    if (self == NULL || id == 0)
        return;

    for (iter = g_list_first (self->active_patterns); iter; iter = g_list_next (iter)) {
        p = (Pattern*) iter->data;
        if (p->id == id) {
            self->active_patterns = g_list_remove (self->active_patterns, p);
            _toggle_pattern (self, p->pattern, FALSE);

            g_free (p->pattern);
            g_free (p);
            break;
        }
    }
}
