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

#ifndef AUDIO_H
#define AUDIO_H

#include <glib.h>

#include "controller.h"
#include "audio-stream.h"

typedef struct _Audio Audio;

Audio*       audio_create              ();
void         audio_destroy             (Audio *self);
void         audio_set_volume          (Audio *self, const char *role, gint volume);
void         audio_register_controller (Audio *self, const char *name, const char *pattern, gboolean repeat);
Controller*  audio_get_controller      (Audio *self, const char *name);

AudioStream* audio_create_stream       (Audio *self, AudioStreamType type);
void         audio_destroy_stream      (Audio *self, AudioStream *stream);
gboolean     audio_prepare             (Audio *self, AudioStream *stream);
gboolean     audio_play                (Audio *self, AudioStream *stream);
void         audio_stop                (Audio *self, AudioStream *stream);

#endif /* AUDIO_H */
