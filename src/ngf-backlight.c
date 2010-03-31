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

#include "ngf-controller.h"
#include "ngf-backlight.h"

typedef struct _BacklightPattern
{
    gchar         *name;
    NgfController *controller;
} BacklightPattern;

typedef struct _BacklightActive
{
    guint             id;
    BacklightPattern *pattern;
    NgfBacklight     *backlight;
} BacklightActive;

struct _NgfBacklight
{
    DBusConnection *connection;
    GHashTable     *patterns;
    guint           pattern_count;
    GSList         *active_patterns;
};

static BacklightPattern*
_pattern_new (const char *name,
              const char *pattern,
              gboolean    repeat)
{
    BacklightPattern *p = NULL;

    p = g_slice_new0 (BacklightPattern);
    p->name       = g_strdup (name);
    p->controller = ngf_controller_new_from_string (pattern, repeat);

    if (p->controller == NULL) {
        g_free (p->name);
        g_slice_free (BacklightPattern, p);
        return NULL;
    }

    return p;
}

static void
_pattern_free (BacklightPattern *pattern)
{
    if (pattern == NULL)
        return;

    if (pattern->controller != NULL) {
        ngf_controller_free (pattern->controller);
        pattern->controller = NULL;
    }

    g_free (pattern->name);
    g_slice_free (BacklightPattern, pattern);
}

NgfBacklight*
ngf_backlight_create ()
{
    NgfBacklight *self = NULL;
    DBusError error;

    if ((self = g_new0 (NgfBacklight, 1)) == NULL)
        goto failed;

    if ((self->patterns = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) _pattern_free)) == NULL)
        goto failed;

    dbus_error_init (&error);
    if ((self->connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error)) == NULL) {
        if (dbus_error_is_set (&error))
            dbus_error_free (&error);

        goto failed;
    }

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

    if (self->patterns) {
        g_hash_table_destroy (self->patterns);
        self->patterns = NULL;
    }

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
ngf_backlight_register (NgfBacklight *self,
                        const char   *name,
                        const char   *pattern,
                        gboolean      repeat)
{
    BacklightPattern *p = NULL;

    if (self == NULL)
        return;

    p = _pattern_new (name, pattern, repeat);
    if (p == NULL)
        return;

    g_hash_table_insert (self->patterns, g_strdup (name), p);
}

static gboolean
_controller_cb (NgfController *controller,
                guint          id,
                guint          step_time,
                guint          step_value,
                gpointer       userdata)
{
    BacklightActive *active = (BacklightActive*) userdata;
    _toggle_state (active->backlight, step_value);
    return TRUE;
}

guint
ngf_backlight_start (NgfBacklight *self,
                     const char   *name)
{
    BacklightPattern *pattern       = NULL;
    BacklightActive  *active        = NULL;
    guint             controller_id = 0;

    if (self == NULL || name == NULL)
        return 0;

    pattern = (BacklightPattern*) g_hash_table_lookup (self->patterns, name);
    if (pattern == NULL)
        return 0;

    active = g_slice_new0 (BacklightActive);
    self->active_patterns = g_slist_append (self->active_patterns, active);

    active->pattern   = pattern;
    active->backlight = self;
    active->id        = ngf_controller_start (active->pattern->controller, _controller_cb, active);

    if (active->id == 0) {
        self->active_patterns = g_slist_remove (self->active_patterns, active);
        g_slice_free (BacklightActive, active);
        return 0;
    }

    return active->id;
}

void
ngf_backlight_stop (NgfBacklight *self,
                    guint         id)
{
    BacklightActive *active = NULL;
    GSList          *iter   = NULL;

    if (self == NULL || id == 0)
        return;

    for (iter = self->active_patterns; iter; iter = g_slist_next (iter)) {
        active = (BacklightActive*) iter->data;
        if (active->id == id) {
            self->active_patterns = g_slist_remove (self->active_patterns, active);
            ngf_controller_stop (active->pattern->controller, active->id);
            g_slice_free (BacklightActive, active);
            break;
        }
    }
}
