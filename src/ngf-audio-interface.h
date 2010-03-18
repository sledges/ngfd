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

#ifndef NGF_AUDIO_INTERFACE_H
#define NGF_AUDIO_INTERFACE_H

#include <glib.h>

#include "ngf-pulse-context.h"
#include "ngf-audio-stream.h"

typedef struct _NgfAudioInterface NgfAudioInterface;

struct _NgfAudioInterface
{
    gboolean (*initialize) (NgfAudioInterface *iface, NgfPulseContext *context);
    void     (*shutdown)   (NgfAudioInterface *iface);
    gboolean (*prepare)    (NgfAudioInterface *iface, NgfAudioStream *stream);
    gboolean (*play)       (NgfAudioInterface *iface, NgfAudioStream *stream);
    void     (*stop)       (NgfAudioInterface *iface, NgfAudioStream *stream);

    gpointer data;
};

gboolean        ngf_audio_interface_initialize     (NgfAudioInterface *iface, NgfPulseContext *context);
void            ngf_audio_interface_shutdown       (NgfAudioInterface *iface);
NgfAudioStream* ngf_audio_interface_create_stream  (NgfAudioInterface *iface);
void            ngf_audio_interface_destroy_stream (NgfAudioInterface *iface, NgfAudioStream *stream);
gboolean        ngf_audio_interface_prepare        (NgfAudioInterface *iface, NgfAudioStream *stream);
gboolean        ngf_audio_interface_play           (NgfAudioInterface *iface, NgfAudioStream *stream);
void            ngf_audio_interface_stop           (NgfAudioInterface *iface, NgfAudioStream *stream);

#endif /* NGF_AUDIO_INTERFACE_H */
