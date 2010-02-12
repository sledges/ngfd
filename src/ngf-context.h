#ifndef NGF_CONTEXT_H
#define NGF_CONTEXT_H

#include <glib.h>

#include "ngf-profile.h"
#include "ngf-audio.h"
#include "ngf-vibrator.h"
#include "ngf-tonegen.h"

typedef struct _NgfContext
{
    NgfProfile      *profile;
    NgfAudio        *audio;
    NgfVibrator     *vibrator;
    NgfTonegen      *tonegen;
} NgfContext;

#endif /* NGF_CONTEXT_H */
