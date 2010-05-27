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

#include <stdio.h>
#include <ImmVibe.h>
#include <ImmVibeCore.h>

#include "vibrator.h"

struct _Vibrator
{
    VibeInt32   device;
};

Vibrator*
vibrator_create ()
{
    Vibrator *self = NULL;

    if ((self = g_new0 (Vibrator, 1)) == NULL)
        goto failed;

    if (VIBE_FAILED (ImmVibeInitialize (VIBE_CURRENT_VERSION_NUMBER)))
        goto failed;

    if (VIBE_FAILED (ImmVibeOpenDevice (0, &self->device)))
        goto failed;

    return self;

failed:
    vibrator_destroy (self);
    return NULL;
}

void
vibrator_destroy (Vibrator *self)
{
    if (self == NULL)
        return;

    if (self->device != VIBE_INVALID_DEVICE_HANDLE_VALUE) {
        ImmVibeStopAllPlayingEffects (self->device);
        ImmVibeCloseDevice (self->device);
        self->device = VIBE_INVALID_DEVICE_HANDLE_VALUE;
        ImmVibeTerminate ();
    }

    g_free (self);
}

gpointer
vibrator_load (const char *filename)
{
    FILE *fp = NULL;
    long pattern_size = 0;
    size_t bytes_read = 0;
    VibeUInt8 *data = NULL;

    if (filename == NULL)
        goto failed;

    if ((fp = fopen (filename, "rb")) == NULL)
        goto failed;

    fseek (fp, 0L, SEEK_END);
    pattern_size = ftell (fp);
    fseek (fp, 0L, SEEK_SET);

    if (pattern_size > 0 && ((data = g_new (VibeUInt8, pattern_size)) != NULL)) {
        bytes_read = fread (data, sizeof (VibeUInt8), pattern_size, fp);
        if (bytes_read != pattern_size)
            goto failed;

        fclose (fp);

        return (gpointer)data;
    }

failed:
    if (data) {
        g_free (data);
        data = NULL;
    }

    if (fp) {
        fclose (fp);
        fp = NULL;
    }

    return NULL;
}

guint
vibrator_start (Vibrator *self, gpointer data, gint pattern_id)
{
    VibeUInt8 *effects = data ? (VibeUInt8*) data : g_pVibeIVTBuiltInEffects;
    gint       id = 0;

    if (self == NULL)
        return 0;

    if (VIBE_SUCCEEDED (ImmVibePlayIVTEffect (self->device, effects, pattern_id, &id)))
        return id;

    return 0;
}

void
vibrator_stop (Vibrator *self, gint id)
{
    VibeStatus status       = 0;
    VibeInt32  effect_state = 0;

    if (self == NULL || id < 0)
        return;

    status = ImmVibeGetEffectState (self->device, id, &effect_state);
    if (VIBE_SUCCEEDED (status)) {
        ImmVibeStopPlayingEffect (self->device, id);
    }
}

gboolean
vibrator_is_completed (Vibrator *self, gint id)
{
    VibeStatus status;
    VibeInt32 effect_state = 0;

    status = ImmVibeGetEffectState (self->device, id, &effect_state);
    if (VIBE_SUCCEEDED (status)) {
        if (status == VIBE_EFFECT_STATE_PLAYING)
            return FALSE;
    }

    return TRUE;
}

gboolean
vibrator_is_repeating (Vibrator *self, gpointer data, gint pattern_id)
{
    VibeUInt8 *effects  = data ? (VibeUInt8*) data : g_pVibeIVTBuiltInEffects;
    VibeInt32  duration = 0;

    if (self == NULL)
        return FALSE;

    ImmVibeGetIVTEffectDuration (effects, pattern_id, &duration);
    if (duration == VIBE_TIME_INFINITE)
        return TRUE;

    return FALSE;
}
