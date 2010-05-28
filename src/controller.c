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
free_active_controller (ActiveController *active)
{
    if (active->source_id > 0) {
        g_source_remove (active->source_id);
        active->source_id = 0;
    }

    g_slice_free (ActiveController, active);
}

static ActiveController*
find_active_controller (Controller *controller, guint id)
{
    GList            *iter   = NULL;
    ActiveController *active = NULL;

    for (iter = g_list_first (controller->active_controllers); iter; iter = g_list_next (iter)) {
        active = (ActiveController*) iter->data;
        if (active->id == id)
            return active;
    }

    return NULL;
}

static gboolean
process_next_step_cb (gpointer userdata)
{
    ActiveController *active     = (ActiveController*) userdata;
    Controller       *controller = active->controller;
    ControllerStep   *step       = NULL;
    GList            *next       = NULL;
    gboolean          is_last    = FALSE;
    guint             old_time   = 0;
    guint             old_value  = 0;

    /* if we have no next event, this is the last step. free the controller. */

    if ((next = g_list_next (active->current)) == NULL)
        is_last = TRUE;

    /* store the current step data. */

    step      = (ControllerStep*) active->current->data;
    old_time  = step->time;
    old_value = step->value;

    /* if the step was last, then free the controller. */

    if (is_last) {
        controller->active_controllers = g_list_remove (controller->active_controllers, active);
        free_active_controller (active);
        return FALSE;
    }
    else {
        /* schedule a next step. if next step is a repeat event, then the next step is
           the first one. */

        step = (ControllerStep*) next->data;
        active->current   = step->repeat ? g_list_first (controller->steps) : next;
        active->source_id = g_timeout_add ((step->time - old_time), process_next_step_cb, active);
    }

    /* call the user set callback with the current step data. */

    if (active->callback)
        active->callback (active->controller, old_time, old_value, is_last, active->userdata);

    return FALSE;
}

Controller*
controller_new ()
{
    return g_new0 (Controller, 1);
}

Controller*
controller_new_from_string (const char *pattern)
{
    Controller  *c          = NULL;
    gchar      **split      = NULL;
    guint        step_time  = 0;
    guint        step_value = 0;

    if (pattern == NULL)
        return NULL;

    c = controller_new ();

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

        if (g_str_equal (*item, "repeat"))
            controller_add_step (c, step_time, 0, TRUE);
        else {
            step_value = atoi (*item);
            controller_add_step (c, step_time, step_value, FALSE);
        }

        ++item;
    }

    g_strfreev (split);

    return c;

failed:
    controller_free (c);
    return NULL;
}

void
controller_free (Controller *controller)
{
    GList            *iter   = NULL;
    ActiveController *active = NULL;
    ControllerStep   *step   = NULL;

    if (controller == NULL)
        return;

    for (iter = g_list_first (controller->active_controllers); iter; iter = g_list_next (iter)) {
        active = (ActiveController*) iter->data;
        free_active_controller (active);
    }

    g_list_free (controller->active_controllers);
    controller->active_controllers = NULL;

    for (iter = g_list_first (controller->steps); iter; iter = g_list_next (iter)) {
        step = (ControllerStep*) iter->data;
        g_slice_free (ControllerStep, step);
    }

    g_list_free (controller->steps);
    controller->steps = NULL;

    g_free (controller);
}

void
controller_add_step (Controller *controller, guint step_time, guint step_value, gboolean repeat)
{
    ControllerStep *step = NULL;

    if (controller == NULL)
        return;

    step         = g_slice_new0 (ControllerStep);
    step->time   = step_time;
    step->value  = step_value;
    step->repeat = repeat;

    controller->steps = g_list_append (controller->steps, step);
}

GList*
controller_get_steps (Controller *controller)
{
    if (controller == NULL)
        return NULL;

    return controller->steps;
}

guint
controller_start (Controller *controller, ControllerCallback callback, gpointer userdata)
{
    ActiveController *active = NULL;
    ControllerStep   *step   = NULL;

    if (controller == NULL || callback == NULL)
        return 0;

    if (controller->steps == NULL)
        return 0;

    active = g_slice_new0 (ActiveController);
    active->current    = g_list_first (controller->steps);
    active->repeat     = controller->repeat;
    active->controller = controller;
    active->callback   = callback;
    active->userdata   = userdata;
    active->id         = ++controller->controller_count;

    if (!active->current) {
        free_active_controller (active);
        return 0;
    }

    controller->active_controllers = g_list_append (controller->active_controllers, active);

    step = (ControllerStep*) active->current;
    if (step->time == 0)
        process_next_step_cb (active);
    else
        active->source_id = g_timeout_add (step->time, process_next_step_cb, active);

    return active->id;
}

void
controller_stop (Controller *controller, guint id)
{
    GList            *iter   = NULL;
    ActiveController *active = NULL;

    if (controller == NULL || id == 0)
        return;

    active = find_active_controller (controller, id);
    if (!active)
        return;

    controller->active_controllers = g_list_remove (controller->active_controllers, active);
    free_active_controller (active);
}
