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

#ifndef NGF_AUDIO_H
#define NGF_AUDIO_H

#include <glib.h>
#include <pulse/proplist.h>

typedef struct _NgfAudio NgfAudio;

typedef enum _NgfAudioState
{
    NGF_AUDIO_READY = 0,
    NGF_AUDIO_FAILED,
    NGF_AUDIO_TERMINATED,
    NGF_AUDIO_SAMPLE_LIST
} NgfAudioState;

typedef enum _NgfStreamState
{
    NGF_STREAM_STARTED = 0,
    NGF_STREAM_COMPLETED,
    NGF_STREAM_STOPPED,
    NGF_STREAM_FAILED,
    NGF_STREAM_TERMINATED
} NgfStreamState;

typedef void (*NgfAudioCallback) (NgfAudio *audio, NgfAudioState state, gpointer user_data);
typedef void (*NgfStreamCallback) (NgfAudio *audio, guint stream_id, NgfStreamState state, gpointer user_data);

NgfAudio*   ngf_audio_create ();
void        ngf_audio_destroy (NgfAudio *self);
void        ngf_audio_set_callback (NgfAudio *self, NgfAudioCallback callback, gpointer userdata);
void        ngf_audio_set_volume (NgfAudio *self, const char *role, gint volume);

guint       ngf_audio_play_stream (NgfAudio *self, const char *filename, pa_proplist *p, NgfStreamCallback callback, gpointer userdata);
void        ngf_audio_stop_stream (NgfAudio *self, guint stream_id);

GList*      ngf_audio_get_sample_list (NgfAudio *self);
void        ngf_audio_free_sample_list (GList *sample_list);

#endif /* NGF_AUDIO_H */
