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

#ifndef AUDIO_H
#define AUDIO_H

#include <glib.h>

#include "audio-stream.h"

typedef struct _Audio Audio;

Audio*       audio_create              ();
void         audio_destroy             (Audio *self);
void         audio_set_volume          (Audio *self, const char *role, gint volume);

AudioStream* audio_create_stream       (Audio *self, AudioStreamType type);
void         audio_destroy_stream      (Audio *self, AudioStream *stream);
gboolean     audio_prepare             (Audio *self, AudioStream *stream);
gboolean     audio_play                (Audio *self, AudioStream *stream);
void         audio_stop                (Audio *self, AudioStream *stream);

#endif /* AUDIO_H */
