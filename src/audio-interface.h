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

#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include <glib.h>

#include "pulse-context.h"
#include "audio-stream.h"

typedef struct _AudioInterface AudioInterface;

struct _AudioInterface
{
    gboolean (*initialize) (AudioInterface *iface, PulseContext *context);
    void     (*shutdown)   (AudioInterface *iface);
    gboolean (*prepare)    (AudioInterface *iface, AudioStream *stream);
    gboolean (*play)       (AudioInterface *iface, AudioStream *stream);
    void     (*stop)       (AudioInterface *iface, AudioStream *stream);

    gpointer data;
};

gboolean        audio_interface_initialize     (AudioInterface *iface, PulseContext *context);
void            audio_interface_shutdown       (AudioInterface *iface);
AudioStream*    audio_interface_create_stream  (AudioInterface *iface);
void            audio_interface_destroy_stream (AudioInterface *iface, AudioStream *stream);
gboolean        audio_interface_prepare        (AudioInterface *iface, AudioStream *stream);
gboolean        audio_interface_play           (AudioInterface *iface, AudioStream *stream);
void            audio_interface_stop           (AudioInterface *iface, AudioStream *stream);

#endif /* AUDIO_INTERFACE_H */
