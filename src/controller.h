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
