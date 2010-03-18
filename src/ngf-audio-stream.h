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

#ifndef NGF_AUDIO_STREAM_H
#define NGF_AUDIO_STREAM_H

#include <glib.h>
#include <pulse/proplist.h>

typedef struct _NgfAudioStream      NgfAudioStream;
typedef enum   _NgfAudioStreamState NgfAudioStreamState;
typedef enum   _NgfAudioStreamType  NgfAudioStreamType;

typedef void (*NgfAudioStreamCallback) (NgfAudioStream *stream, NgfAudioStreamState state, gpointer userdata);

enum _NgfAudioStreamState
{
    NGF_AUDIO_STREAM_STATE_NONE,
    NGF_AUDIO_STREAM_STATE_STARTED,
    NGF_AUDIO_STREAM_STATE_COMPLETED,
    NGF_AUDIO_STREAM_STATE_FAILED
};

enum _NgfAudioStreamType
{
    NGF_AUDIO_STREAM_NONE,
    NGF_AUDIO_STREAM_UNCOMPRESSED
};

struct _NgfAudioStream
{
    guint                   id;
    gchar                  *source;
    pa_proplist            *properties;
    NgfAudioStreamCallback  callback;
    gpointer                userdata;

    /** Private implementation data */
    NgfAudioStreamType      type;
    gpointer                iface;
    gpointer                data;
};

#endif /* NGF_AUDIO_STREAM_H */
