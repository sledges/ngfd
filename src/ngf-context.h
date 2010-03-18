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

#ifndef NGF_CONTEXT_H
#define NGF_CONTEXT_H

#include <glib.h>

#include "ngf-profile.h"
#include "ngf-tone-mapper.h"
#include "ngf-audio.h"
#include "ngf-vibrator.h"
#include "ngf-tonegen.h"
#include "ngf-led.h"
#include "ngf-backlight.h"

typedef struct _NgfContext
{
    NgfProfile      *profile;
    NgfToneMapper   *tone_mapper;
    NgfAudio        *audio;
    NgfVibrator     *vibrator;
    NgfTonegen      *tonegen;
    NgfLed          *led;
    NgfBacklight    *backlight;
} NgfContext;

#endif /* NGF_CONTEXT_H */
