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
