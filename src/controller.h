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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <glib.h>

typedef struct _ControllerStep
{
    guint    time;
    guint    value;
    gboolean repeat;
} ControllerStep;

typedef struct _Controller Controller;
typedef void (*ControllerCallback) (Controller *controller, guint step_time, guint step_value, gboolean is_last, gpointer userdata);

Controller* controller_new             ();
Controller* controller_new_from_string (const char *pattern);
void        controller_free            (Controller *self);
void        controller_add_step        (Controller *self, guint step_time, guint step_value, gboolean repeat);
GList*      controller_get_steps       (Controller *self);
guint       controller_start           (Controller *self, ControllerCallback callback, gpointer userdata);
void        controller_stop            (Controller *self, guint id);

#endif /* CONTROLLER_H */
