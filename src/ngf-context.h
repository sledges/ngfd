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
