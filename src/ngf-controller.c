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

#include "ngf-controller.h"

typedef struct _ControlStep ControlStep;
typedef struct _Controller  Controller;

struct _ControlStep
{
    guint time;
    guint value;
};

struct _Controller
{
    guint                   id;
    guint                   source_id;
    gboolean                repeat;
    GList                   *current;
    NgfController           *controller;
    NgfControllerCallback   callback;
    gpointer                userdata;
};

struct _NgfController
{
    GList   *steps;
    GList   *active_controllers;
    guint   controller_count;
};

static void
_free_controller (Controller *c)
{
    g_slice_free (Controller, c);
}

static gboolean
_process_next_step (gpointer userdata)
{
    Controller *c = (Controller*) userdata;
    NgfController *self = c->controller;

    ControlStep *step = NULL, *next = NULL;
    gboolean do_continue = FALSE;

    step = (ControlStep*) c->current->data;
    g_print ("CONTROLLER STEP (id=%d, time=%d, value=%d)\n", c->id, step->time, step->value);

    if (c->callback)
        do_continue = c->callback (c->id, step->time, step->value, c->userdata);

    c->current = g_list_next (c->current);
    if (do_continue && c->current) {
        next = (ControlStep*) c->current->data;
        c->source_id = g_timeout_add ((next->time - step->time), _process_next_step, c);
    }
    else {
        if (c->repeat) {
            g_print ("CONTROLLER REPEAT (id=%d)\n", c->id);

            c->current = g_list_first (self->steps);
            step = (ControlStep*) c->current->data;
            if (step->time == 0)
                _process_next_step ((gpointer) c);
            else
                c->source_id = g_timeout_add (step->time, _process_next_step, c);
        }
        else {
            g_print ("CONTROLLER COMPLETE (id=%d)\n", c->id);
            self->active_controllers = g_list_remove (self->active_controllers, c);
            _free_controller (c);
        }
    }

    return FALSE;
}

NgfController*
ngf_controller_new ()
{
    return g_new0 (NgfController, 1);
}

void
ngf_controller_free (NgfController *self)
{
    GList *iter = NULL;

    Controller *c = NULL;
    ControlStep *step = NULL;

    if (self == NULL)
        return;

    for (iter = g_list_first (self->active_controllers); iter; iter = g_list_next (iter)) {
        c = (Controller*) iter->data;
        _free_controller (c);
    }

    g_list_free (self->active_controllers);
    self->active_controllers = NULL;

    for (iter = g_list_first (self->steps); iter; iter = g_list_next (iter)) {
        step = (ControlStep*) iter->data;
        g_slice_free (ControlStep, step);
    }

    g_list_free (self->steps);
    self->steps = NULL;

    g_free (self);
}

void
ngf_controller_add_step (NgfController *self, guint step_time, guint step_value)
{
    ControlStep *step = NULL;

    if (self == NULL)
        return;

    step = g_slice_new0 (ControlStep);
    step->time = step_time;
    step->value = step_value;

    self->steps = g_list_append (self->steps, step);
}

guint
ngf_controller_start (NgfController *self, gboolean repeat, NgfControllerCallback callback, gpointer userdata)
{
    Controller *c = NULL;
    ControlStep *step = NULL;

    if (self == NULL || callback == NULL)
        return 0;

    if (self->steps == NULL)
        return 0;

    c = g_slice_new0 (Controller);
    c->current      = g_list_first (self->steps);
    c->repeat       = repeat;
    c->controller   = self;
    c->callback     = callback;
    c->userdata     = userdata;
    c->id           = ++self->controller_count;

    if (c->current == NULL)
        return 0;

    /* If the first step's time is 0, then let's execute that immediately. */
    self->active_controllers = g_list_append (self->active_controllers, c);

    step = (ControlStep*) c->current->data;
    if (step->time == 0)
        _process_next_step ((gpointer) c);
    else
        c->source_id = g_timeout_add (step->time, _process_next_step, c);

    return c->id;
}

void
ngf_controller_stop (NgfController *self, guint id)
{
    GList *iter = NULL;
    Controller *c = NULL;

    if (self == NULL || id == 0)
        return;

    for (iter = g_list_first (self->active_controllers); iter; iter = g_list_next (iter)) {
        c = (Controller*) iter->data;

        if (c->id == id) {
            if (c->source_id > 0) {
                g_source_remove (c->source_id);
                c->source_id = 0;
            }

            self->active_controllers = g_list_remove (self->active_controllers, c);
            _free_controller (c);
            break;
        }
    }
}
