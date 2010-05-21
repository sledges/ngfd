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

#include "audio-interface.h"

gboolean
audio_interface_initialize (AudioInterface *iface, PulseContext *context)
{
    return iface->initialize (iface, context);
}

void
audio_interface_shutdown (AudioInterface *iface)
{
    iface->shutdown (iface);
}

AudioStream*
audio_interface_create_stream (AudioInterface *iface)
{
    AudioStream *stream = NULL;

    stream = g_slice_new0 (AudioStream);
    stream->iface = (gpointer) iface;
    return stream;
}

void
audio_interface_destroy_stream (AudioInterface *iface, AudioStream *stream)
{
    if (iface == NULL || stream == NULL)
        return;

    if (stream->properties) {
        pa_proplist_free (stream->properties);
        stream->properties = NULL;
    }

    g_free (stream->source);
    g_slice_free (AudioStream, stream);
}

gboolean
audio_interface_prepare (AudioInterface *iface, AudioStream *stream)
{
    return iface->prepare (iface, stream);
}

gboolean
audio_interface_play (AudioInterface *iface, AudioStream *stream)
{
    return iface->play (iface, stream);
}

void
audio_interface_stop (AudioInterface *iface, AudioStream *stream)
{
    iface->stop (iface, stream);
}
