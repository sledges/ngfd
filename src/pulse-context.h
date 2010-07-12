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

#ifndef PULSE_CONTEXT_H
#define PULSE_CONTEXT_H

#include <glib.h>
#include <pulse/context.h>

typedef enum _PulseContextState
{
    /** Unknown or initial state. */
    PULSE_CONTEXT_STATE_UNKNOWN = 0,

    /** Context is being set up and it is not ready yet. */
    PULSE_CONTEXT_STATE_SETUP,

    /** Context is ready for operations. */
    PULSE_CONTEXT_STATE_READY,

    /** Context timed out, we couldn't connect within certain time period. */
    PULSE_CONTEXT_STATE_TIMEOUT,

    /** Failed to connect to Pulseaudio or error occurred when creating context. */
    PULSE_CONTEXT_STATE_FAILED,

    /** Context lost connection to Pulseaudio. */
    PULSE_CONTEXT_STATE_TERMINATED
} PulseContextState;

/** PulseContext declaration */
typedef struct _PulseContext PulseContext;

/** Callback for getting notified of Pulseaudio context state */
typedef void (*PulseContextCallback) (PulseContext *context, PulseContextState state, gpointer userdata);

PulseContext* pulse_context_create       ();
void          pulse_context_destroy      (PulseContext *self);
void          pulse_context_set_callback (PulseContext *self, PulseContextCallback callback, gpointer userdata);
pa_context*   pulse_context_get_context  (PulseContext *self);
void          pulse_context_set_volume   (PulseContext *self, const char *role, gint volume);

#endif /* PULSE_CONTEXT_H */
