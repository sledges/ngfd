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

#include "ngf-pulse-context.h"
#include "ngf-audio-gstreamer.h"
#include "ngf-audio-pulse.h"
#include "ngf-audio-stream.h"
#include "ngf-audio.h"

struct _NgfAudio
{
    NgfPulseContext   *context;
    NgfAudioInterface *gst;
    NgfAudioInterface *pulse;
};

NgfAudio*
ngf_audio_create ()
{
    NgfAudio *self = NULL;

    if ((self = g_new0 (NgfAudio, 1)) == NULL)
        goto failed;

    if ((self->context = ngf_pulse_context_create ()) == NULL)
        goto failed;

    self->gst = ngf_audio_gstreamer_create ();
    if (!ngf_audio_interface_initialize (self->gst, self->context))
        goto failed;

    self->pulse = ngf_audio_pulse_create ();
    if (!ngf_audio_interface_initialize (self->pulse, self->context))
        goto failed;

    return self;

failed:
    ngf_audio_destroy (self);
    return NULL;
}

void
ngf_audio_destroy (NgfAudio *self)
{
    if (self == NULL)
        return;

    if (self->pulse) {
        ngf_audio_interface_shutdown (self->pulse);
        self->pulse = NULL;
    }

    if (self->gst) {
        ngf_audio_interface_shutdown (self->gst);
        self->gst = NULL;
    }

    if (self->context) {
        ngf_pulse_context_destroy (self->context);
        self->context = NULL;
    }

    g_free (self);
}

void
ngf_audio_set_volume (NgfAudio *self, const char *role, gint volume)
{
    if (self == NULL || role == NULL || volume <= 0)
        return;

    ngf_pulse_context_set_volume (self->context, role, volume);
}

NgfAudioStream*
ngf_audio_create_stream (NgfAudio *self, NgfAudioStreamType type)
{
    NgfAudioStream    *stream = NULL;
    NgfAudioInterface *iface  = NULL;

    if (self == NULL)
        return NULL;

    iface = (type == NGF_AUDIO_STREAM_UNCOMPRESSED) ? self->pulse : self->gst;

    stream = ngf_audio_interface_create_stream (iface);
    stream->type = type;

    return stream;
}

void
ngf_audio_destroy_stream (NgfAudio *self, NgfAudioStream *stream)
{
    NgfAudioInterface *iface = NULL;

    if (self == NULL || stream == NULL)
        return;

    iface = (NgfAudioInterface*) stream->iface;
    ngf_audio_interface_destroy_stream (iface, stream);
}

gboolean
ngf_audio_prepare (NgfAudio *self, NgfAudioStream *stream)
{
    if (self == NULL || stream == NULL)
        return FALSE;

    if (!ngf_audio_interface_prepare (stream->iface, stream))
        return FALSE;

    return TRUE;
}

gboolean
ngf_audio_play (NgfAudio *self, NgfAudioStream *stream)
{
    if (self == NULL || stream == NULL)
        return FALSE;

    if (!ngf_audio_interface_play (stream->iface, stream))
        return FALSE;

    return TRUE;
}

void
ngf_audio_stop (NgfAudio *self, NgfAudioStream *stream)
{
    if (self == NULL || stream == NULL)
        return;

    ngf_audio_interface_stop (stream->iface, stream);
}
