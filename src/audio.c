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
    GHashTable        *controllers;
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

    if ((self->controllers = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) controller_free)) == NULL)
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

    if (self->controllers) {
        g_hash_table_destroy (self->controllers);
        self->controllers = NULL;
    }

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

void
audio_register_controller (Audio *self, const char *name, const char *pattern, gboolean repeat)
{
    Controller *c = NULL;

    if (self == NULL || name == NULL || pattern == NULL)
        return;

    if ((c = controller_new_from_string (pattern, repeat)) == NULL)
        return;

    g_hash_table_insert (self->controllers, g_strdup (name), c);
}

Controller*
audio_get_controller (Audio *self, const char *name)
{
    if (self == NULL || name == NULL)
        return NULL;

    return (Controller*) g_hash_table_lookup (self->controllers, name);
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
