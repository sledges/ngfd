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

#include "ngf-event-prototype.h"

NgfEventPrototype*
ngf_event_prototype_new ()
{
    NgfEventPrototype *proto = NULL;

    if ((proto = g_new0 (NgfEventPrototype, 1)) == NULL)
        return NULL;

    return proto;
}

void
ngf_event_prototype_free (NgfEventPrototype *proto)
{
    if (proto == NULL)
        return;

    g_free (proto->audio_tone_filename);
    g_free (proto->audio_tone_key);
    g_free (proto->audio_tone_profile);
    g_free (proto->audio_fallback_filename);
    g_free (proto->audio_fallback_key);
    g_free (proto->audio_fallback_profile);
    g_free (proto->audio_volume_key);
    g_free (proto->audio_volume_profile);
    g_free (proto->audio_stream_role);
    g_free (proto->vibrator_pattern);
    g_free (proto->led_pattern);

    if (proto->audio_volume_controller) {
        ngf_controller_free (proto->audio_volume_controller);
        proto->audio_volume_controller = NULL;
    }

    if (proto->backlight_controller) {
        ngf_controller_free (proto->backlight_controller);
        proto->backlight_controller = NULL;
    }

    if (proto->stream_properties) {
        pa_proplist_free (proto->stream_properties);
        proto->stream_properties = NULL;
    }

    g_free (proto);
}
