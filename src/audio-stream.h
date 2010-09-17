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

#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <glib.h>
#include <gst/gst.h>
#include <gst/controller/gstcontroller.h>
#include "volume.h"

typedef struct _AudioStream      AudioStream;
typedef enum   _AudioStreamState AudioStreamState;
typedef enum   _AudioStreamType  AudioStreamType;

typedef void (*AudioStreamCallback) (AudioStream *stream, AudioStreamState state, gpointer userdata);

enum _AudioStreamState
{
    AUDIO_STREAM_STATE_NONE,
    AUDIO_STREAM_STATE_PREPARED,
    AUDIO_STREAM_STATE_STARTED,
    AUDIO_STREAM_STATE_REWIND,
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
    GstStructure           *properties;
    gint                    buffer_time;
    gint                    latency_time;
    AudioStreamCallback     callback;
    gpointer                userdata;
    Volume                  *volume;
    gdouble                 time_played;

    /** Private implementation data */
    AudioStreamType         type;
    gpointer                iface;
    GstElement              *pipeline;
    GstController           *controller;
    GstInterpolationControlSource *csource;
    gboolean                repeating;
    guint                   num_repeats;
    guint                   current_repeat;
    GstElement              *volume_element;
};

#endif /* AUDIO_STREAM_H */
