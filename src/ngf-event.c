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

#include "ngf-value.h"
#include "ngf-event.h"

static gboolean     _event_max_timeout_cb (gpointer userdata);
static void         _stream_state_cb (NgfAudio *audio, guint stream_id, NgfStreamState state, gpointer userdata);
static const char*  _event_get_tone (NgfEvent *self);
static const char*  _event_stop_audio (NgfEvent *self);



NgfEvent*
ngf_event_new (NgfContext *context, NgfEventPrototype *proto)
{
    NgfEvent *self = NULL;

    if (context == NULL || proto == NULL)
        return NULL;

    if ((self = g_new0 (NgfEvent, 1)) == NULL)
        return NULL;

    self->context = context;
    self->proto = proto;
    self->start_timer = g_timer_new ();

    return self;
}

void
ngf_event_free (NgfEvent *self)
{
    if (self == NULL)
        return;

    if (self->start_timer) {
        g_timer_destroy (self->start_timer);
        self->start_timer = NULL;
    }

    g_free (self);
}

void
ngf_event_set_callback (NgfEvent *self, NgfEventCallback callback, gpointer userdata)
{
    self->callback = callback;
    self->userdata = userdata;
}

static gboolean
_event_max_timeout_cb (gpointer userdata)
{
    NgfEvent *self = (NgfEvent*) userdata;
    g_print ("max_timeout reached\n");
    ngf_event_stop (self);

    return FALSE;
}

static const char*
_event_get_tone (NgfEvent *self)
{
    NgfValue *value = NULL;
    
    value = g_hash_table_lookup (self->properties, "audio");
    if (value && ngf_value_get_type (value) == NGF_VALUE_STRING)
        return (const char*) ngf_value_get_string (value);

    if (self->proto->audio_filename)
        return self->proto->audio_filename;

    if (self->proto->audio_tone_key)
        return NULL;
        /*return ngf_profile_get_string (self->context->profile, self->proto->audio_tone_key);*/

    return NULL;
}

static void
_stream_state_cb (NgfAudio *audio, guint stream_id, NgfStreamState state, gpointer userdata)
{
    NgfEvent *self = (NgfEvent*) userdata;

    switch (state) {
        case NGF_STREAM_STARTED: {

            /* Stream started, let's notify the client that we are all up and running */

            if (self->callback)
                self->callback (self, NGF_EVENT_STARTED, self->userdata);

            break;
        }

        case NGF_STREAM_FAILED: {

            /* Stream failed to start, most probably reason is that the file
               does not exist. In this case, if the fallback is specified we will
               try to play it. */

            if (self->callback)
                self->callback (self, NGF_EVENT_FAILED, self->userdata);

            break;
        }

        case NGF_STREAM_TERMINATED: {

            /* Audio stream terminated. This means that the underlying audio context
               was lost (probably Pulseaudio went down). We will stop the event at
               this point (consider it completed). */

            if (self->callback)
                self->callback (self, NGF_EVENT_COMPLETED, self->userdata);

            break;
        }

        case NGF_STREAM_COMPLETED: {

            /* Stream was played and completed successfully. If the repeat flag is
               set, then we will restart the stream again. Otherwise, let's notify
               the user of the completion of the event. */

            if (self->callback)
                self->callback (self, NGF_EVENT_COMPLETED, self->userdata);

            break;
        }

        default:
            break;
    }
}

gboolean
ngf_event_start (NgfEvent *self)
{
    const char *tone = NULL;

    /* Check the resources and start the backends if we have the proper resources,
       profile allows us to and valid data is provided. */

    if (self->resources & NGF_RESOURCE_AUDIO) {

        if (self->play_mode == NGF_PLAY_MODE_LONG) {
            if ((tone = _event_get_tone (self)) != NULL)
                self->audio_id = ngf_audio_play_stream (self->context->audio, tone, _stream_state_cb, self);
        }

    }

    /* Timeout callback for maximum length of the event. Once triggered we will
       stop the event ourselves. */

    if (self->proto->event_max_length > 0)
        self->max_length_timeout_id = g_timeout_add (self->proto->event_max_length, _event_max_timeout_cb, self);

    /* Trigger the start timer, which will be used to monitor the minimum timeout. */

    g_timer_start (self->start_timer);

    return TRUE;
}

void
ngf_event_stop (NgfEvent *self)
{
    if (self->max_length_timeout_id > 0) {
        g_source_remove (self->max_length_timeout_id);
        self->max_length_timeout_id = 0;
    }

    if (self->audio_id > 0) {
        ngf_audio_stop_stream (self->context->audio, self->audio_id);
        self->audio_id = 0;
    }

    if (self->callback)
        self->callback (self, NGF_EVENT_COMPLETED, self->userdata);
}
