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
    return g_new0 (NgfEventPrototype, 1);
}

void
ngf_event_prototype_free (NgfEventPrototype *proto)
{
    if (proto == NULL)
        return;

    g_free (proto->audio_filename);
    g_free (proto->audio_fallback);

    g_free (proto->audio_tone_key);
    g_free (proto->audio_fallback_key);
    g_free (proto->audio_volume_key);
    g_free (proto->audio_stream_restore);

    g_free (proto);
}
