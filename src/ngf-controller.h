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

#ifndef NGF_CONTROLLER_H
#define NGF_CONTROLLER_H

#include <glib.h>

typedef struct _NgfControllerStep
{
    guint time;
    guint value;
} NgfControllerStep;

typedef struct _NgfController NgfController;

typedef gboolean (*NgfControllerCallback) (NgfController *controller, guint id, guint step_time, guint step_value, gpointer userdata);

NgfController*  ngf_controller_new             ();
NgfController*  ngf_controller_new_from_string (const char *pattern, gboolean repeat);
void            ngf_controller_free            (NgfController *self);
void            ngf_controller_add_step        (NgfController *self, guint step_time, guint step_value);
GList*          ngf_controller_get_steps       (NgfController *self);
guint           ngf_controller_start           (NgfController *self, NgfControllerCallback callback, gpointer userdata);
void            ngf_controller_stop            (NgfController *self, guint id);

#endif /* NGF_CONTROLLER_H */
