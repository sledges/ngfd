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

#include "ngf-controller.h"
#include "ngf-audio-stream.h"

typedef struct _NgfAudio NgfAudio;

NgfAudio*        ngf_audio_create              ();
void             ngf_audio_destroy             (NgfAudio *self);
void             ngf_audio_set_volume          (NgfAudio *self, const char *role, gint volume);
void             ngf_audio_register_controller (NgfAudio *self, const char *name, const char *pattern, gboolean repeat);
NgfController*   ngf_audio_get_controller      (NgfAudio *self, const char *name);

NgfAudioStream*  ngf_audio_create_stream    (NgfAudio *self, NgfAudioStreamType type);
void             ngf_audio_destroy_stream   (NgfAudio *self, NgfAudioStream *stream);
gboolean         ngf_audio_prepare          (NgfAudio *self, NgfAudioStream *stream);
gboolean         ngf_audio_play             (NgfAudio *self, NgfAudioStream *stream);
void             ngf_audio_stop             (NgfAudio *self, NgfAudioStream *stream);

#endif /* NGF_AUDIO_H */
