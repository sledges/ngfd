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

#include "pulse-context.h"
#include "audio-pulse.h"
#include "audio-stream.h"
#include "audio.h"
#include "config.h"

#ifdef HAVE_GST
#include "audio-gstreamer.h"
#endif

struct _Audio
{
    PulseContext   *context;
    AudioInterface *gst;
    AudioInterface *pulse;
};

Audio*
audio_create ()
{
    Audio *self = NULL;

    if ((self = g_new0 (Audio, 1)) == NULL)
        goto failed;

    if ((self->context = pulse_context_create ()) == NULL)
        goto failed;

#ifdef HAVE_GST
    self->gst = audio_gstreamer_create ();
    if (!audio_interface_initialize (self->gst, self->context))
        goto failed;
#endif

    self->pulse = audio_pulse_create ();
    if (!audio_interface_initialize (self->pulse, self->context))
        goto failed;

    return self;

failed:
    audio_destroy (self);
    return NULL;
}

void
audio_destroy (Audio *self)
{
    if (self == NULL)
        return;

    if (self->pulse) {
        audio_interface_shutdown (self->pulse);
        self->pulse = NULL;
    }

    if (self->gst) {
        audio_interface_shutdown (self->gst);
        self->gst = NULL;
    }

    if (self->context) {
        pulse_context_destroy (self->context);
        self->context = NULL;
    }

    g_free (self);
}

void
audio_set_volume (Audio *self, const char *role, gint volume)
{
    if (self == NULL || role == NULL || volume < 0)
        return;

    pulse_context_set_volume (self->context, role, volume);
}

AudioStream*
audio_create_stream (Audio *self, AudioStreamType type)
{
    AudioStream    *stream = NULL;
    AudioInterface *iface  = NULL;

    if (self == NULL)
        return NULL;

#ifdef HAVE_GST
    iface = (type == AUDIO_STREAM_UNCOMPRESSED) ? self->pulse : self->gst;
#else
    iface = self->pulse;
#endif

    stream = audio_interface_create_stream (iface);
    stream->type = type;

    return stream;
}

void
audio_destroy_stream (Audio *self, AudioStream *stream)
{
    AudioInterface *iface = NULL;

    if (self == NULL || stream == NULL)
        return;

    iface = (AudioInterface*) stream->iface;
    audio_interface_destroy_stream (iface, stream);
}

gboolean
audio_prepare (Audio *self, AudioStream *stream)
{
    if (self == NULL || stream == NULL)
        return FALSE;

    if (!audio_interface_prepare (stream->iface, stream))
        return FALSE;

    return TRUE;
}

gboolean
audio_play (Audio *self, AudioStream *stream)
{
    if (self == NULL || stream == NULL)
        return FALSE;

    if (!audio_interface_play (stream->iface, stream))
        return FALSE;

    return TRUE;
}

void
audio_stop (Audio *self, AudioStream *stream)
{
    if (self == NULL || stream == NULL)
        return;

    audio_interface_stop (stream->iface, stream);
}
