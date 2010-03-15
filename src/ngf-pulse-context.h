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

#ifndef NGF_PULSE_CONTEXT_H
#define NGF_PULSE_CONTEXT_H

#include <glib.h>
#include <pulse/context.h>

typedef enum _NgfPulseContextState
{
    /** Unknown or initial state. */
    NGF_PULSE_CONTEXT_STATE_UNKNOWN = 0,

    /** Context is being set up and it is not ready yet. */
    NGF_PULSE_CONTEXT_STATE_SETUP,

    /** Context is ready for operations. */
    NGF_PULSE_CONTEXT_STATE_READY,

    /** Context timed out, we couldn't connect within certain time period. */
    NGF_PULSE_CONTEXT_STATE_TIMEOUT,

    /** Failed to connect to Pulseaudio or error occurred when creating context. */
    NGF_PULSE_CONTEXT_STATE_FAILED,

    /** Context lost connection to Pulseaudio. */
    NGF_PULSE_CONTEXT_STATE_TERMINATED
} NgfPulseContextState;

/** NgfPulseContext declaration */
typedef struct _NgfPulseContext NgfPulseContext;

/** Callback for getting notified of Pulseaudio context state */
typedef void (*NgfPulseContextCallback) (NgfPulseContext *context, NgfPulseContextState state, gpointer userdata);

NgfPulseContext* ngf_pulse_context_create       ();
void             ngf_pulse_context_destroy      (NgfPulseContext *self);
void             ngf_pulse_context_set_callback (NgfPulseContext *self, NgfPulseContextCallback callback, gpointer userdata);
pa_context*      ngf_pulse_context_get_context  (NgfPulseContext *self);
void             ngf_pulse_context_set_volume   (NgfPulseContext *self, const char *role, gint volume);

#endif /* NGF_PULSE_CONTEXT_H */
