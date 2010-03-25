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

#ifndef NGF_EVENT_PROTOTYPE_H
#define NGF_EVENT_PROTOTYPE_H

#include <glib.h>
#include <pulse/proplist.h>

#include "ngf-value.h"
#include "ngf-controller.h"

typedef struct _NgfEventPrototype NgfEventPrototype;

struct _NgfEventPrototype
{
    gint             max_length;                /* Maximum event length, once reached the event
                                                   is completed */

    gboolean         audio_enabled;             /* Audio is enabled */
    gboolean         audio_repeat;              /* Enable audio repeating */
    gint             audio_max_repeats;         /* Maximum number of repeats */
    gchar           *audio_tone_filename;       /* Full path to audio file */
    gchar           *audio_tone_key;            /* Profile key for getting filename */
    gchar           *audio_tone_profile;        /* Profile where to use key */
    gboolean         audio_silent;              /* Should play audio in silent mode */

    gchar           *audio_fallback_filename;   /* Fallback filename if we can't play tone */
    gchar           *audio_fallback_key;        /* Fallback key for profile */
    gchar           *audio_fallback_profile;    /* Fallback profile where to use key */

    gint             audio_volume_value;        /* Volume value to set for stream */
    gchar           *audio_volume_key;          /* Profile key for getting audio volume */
    gchar           *audio_volume_profile;      /* What profile volume key applies, NULL for currently active */

    gboolean         audio_volume_repeat;       /* Repeat volume controller once completed */
    NgfController   *audio_volume_controller;   /* Controller for audio volume patterns. */

    gboolean         audio_tonegen_enabled;     /* Tone generator is enabled -- this overrides audio */
    guint            audio_tonegen_pattern;     /* Tone generator pattern */

    gchar           *audio_stream_role;         /* Stream: volume role to use */
    pa_proplist     *stream_properties;         /* Stream: pulseaudio properties to use */

    gboolean         vibrator_enabled;          /* Vibrator is enabled */
    gchar           *vibrator_pattern;          /* Vibrator pattern to use */

    gboolean         led_enabled;               /* Use LED pattern */
    gchar           *led_pattern;               /* LED pattern to use @see /etc/mce/mce.ini */

    gboolean         backlight_enabled;         /* Backlight controller is enabled */
    gboolean         backlight_repeat;          /* Repeat backlight controller pattern */
    NgfController   *backlight_controller;      /* Controller for backlight patterns, if no pattern and
                                                   backlight is enabled, turn on the backlight. */

};

NgfEventPrototype*  ngf_event_prototype_new ();
void                ngf_event_prototype_free (NgfEventPrototype *proto);

#endif /* NGF_EVENT_PROTOTYPE_H */
