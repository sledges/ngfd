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

#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <glib.h>
#include <pulse/proplist.h>

typedef struct _AudioStream      AudioStream;
typedef enum   _AudioStreamState AudioStreamState;
typedef enum   _AudioStreamType  AudioStreamType;

typedef void (*AudioStreamCallback) (AudioStream *stream, AudioStreamState state, gpointer userdata);

enum _AudioStreamState
{
    AUDIO_STREAM_STATE_NONE,
    AUDIO_STREAM_STATE_PREPARED,
    AUDIO_STREAM_STATE_STARTED,
    AUDIO_STREAM_STATE_COMPLETED,
    AUDIO_STREAM_STATE_FAILED
};

enum _AudioStreamType
{
    AUDIO_STREAM_NONE,
    AUDIO_STREAM_UNCOMPRESSED
};

struct _AudioStream
{
    guint                   id;
    gchar                  *source;
    pa_proplist            *properties;
    AudioStreamCallback     callback;
    gpointer                userdata;

    /** Private implementation data */
    AudioStreamType         type;
    gpointer                iface;
    gpointer                data;
};

#endif /* AUDIO_STREAM_H */
