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

#include <stdlib.h>
#include "log.h"
#include "controller.h"

typedef struct _ActiveController ActiveController;

struct _ActiveController
{
    guint               id;
    guint               source_id;
    gboolean            repeat;
    GList              *current;
    Controller         *controller;
    ControllerCallback  callback;
    gpointer            userdata;
};

struct _Controller
{
    GList    *steps;
    GList    *active_controllers;
    gboolean  repeat;
    guint     controller_count;
};

static void
_free_controller (ActiveController *c)
{
    g_slice_free (ActiveController, c);
}

static gboolean
_process_next_step (gpointer userdata)
{
    ActiveController *c = (ActiveController*) userdata;
    Controller *self = c->controller;

    ControllerStep *step = NULL, *next = NULL;
    gboolean do_continue = FALSE;

    step = (ControllerStep*) c->current->data;
    LOG_DEBUG ("CONTROLLER STEP (id=%d, time=%d, value=%d)", c->id, step->time, step->value);

    if (c->callback)
        do_continue = c->callback (c->controller, c->id, step->time, step->value, c->userdata);

    c->current = g_list_next (c->current);
    if (do_continue && c->current) {
        next = (ControllerStep*) c->current->data;
        c->source_id = g_timeout_add ((next->time - step->time), _process_next_step, c);
    }
    else {
        if (c->repeat) {
            c->current = g_list_first (self->steps);
            step = (ControllerStep*) c->current->data;
            if (step->time == 0)
                _process_next_step ((gpointer) c);
            else
                c->source_id = g_timeout_add (step->time, _process_next_step, c);
        }
        else {
            self->active_controllers = g_list_remove (self->active_controllers, c);
            _free_controller (c);
        }
    }

    return FALSE;
}

Controller*
controller_new ()
{
    return g_new0 (Controller, 1);
}

Controller*
controller_new_from_string (const char *pattern, gboolean repeat)
{
    Controller  *c     = NULL;
    gchar         **split = NULL;
    guint           step_time  = 0;
    guint           step_value = 0;

    if (pattern == NULL)
        return NULL;

    c = controller_new ();
    c->repeat = repeat;

    /* Split the pattern by the ; separator */
    split = g_strsplit (pattern, ";", -1);
    if (split == NULL)
        goto failed;

    gchar **item = split;
    while (1) {
        if (*item == NULL)
            break;

        step_time = atoi (*item);

        ++item;
        if (*item == NULL)
            break;

        step_value = atoi (*item);
        controller_add_step (c, step_time, step_value);

        ++item;
    }

    g_strfreev (split);

    return c;

failed:
    controller_free (c);
    return NULL;
}

void
controller_free (Controller *self)
{
    GList *iter = NULL;

    ActiveController *c = NULL;
    ControllerStep *step = NULL;

    if (self == NULL)
        return;

    for (iter = g_list_first (self->active_controllers); iter; iter = g_list_next (iter)) {
        c = (ActiveController*) iter->data;
        _free_controller (c);
    }

    g_list_free (self->active_controllers);
    self->active_controllers = NULL;

    for (iter = g_list_first (self->steps); iter; iter = g_list_next (iter)) {
        step = (ControllerStep*) iter->data;
        g_slice_free (ControllerStep, step);
    }

    g_list_free (self->steps);
    self->steps = NULL;

    g_free (self);
}

void
controller_add_step (Controller *self, guint step_time, guint step_value)
{
    ControllerStep *step = NULL;

    if (self == NULL)
        return;

    step = g_slice_new0 (ControllerStep);
    step->time = step_time;
    step->value = step_value;

    self->steps = g_list_append (self->steps, step);
}

GList*
controller_get_steps (Controller *self)
{
    if (self == NULL)
        return NULL;

    return self->steps;
}

guint
controller_start (Controller *self, ControllerCallback callback, gpointer userdata)
{
    ActiveController *c = NULL;
    ControllerStep *step = NULL;

    if (self == NULL || callback == NULL)
        return 0;

    if (self->steps == NULL)
        return 0;

    c = g_slice_new0 (ActiveController);
    c->current      = g_list_first (self->steps);
    c->repeat       = self->repeat;
    c->controller   = self;
    c->callback     = callback;
    c->userdata     = userdata;
    c->id           = ++self->controller_count;

    if (c->current == NULL)
        return 0;

    /* If the first step's time is 0, then let's execute that immediately. */
    self->active_controllers = g_list_append (self->active_controllers, c);

    step = (ControllerStep*) c->current->data;
    if (step->time == 0)
        _process_next_step ((gpointer) c);
    else
        c->source_id = g_timeout_add (step->time, _process_next_step, c);

    return c->id;
}

void
controller_stop (Controller *self, guint id)
{
    GList *iter = NULL;
    ActiveController *c = NULL;

    if (self == NULL || id == 0)
        return;

    for (iter = g_list_first (self->active_controllers); iter; iter = g_list_next (iter)) {
        c = (ActiveController*) iter->data;

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
