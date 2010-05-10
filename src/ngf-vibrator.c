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

#include "ngf-vibrator.h"

typedef struct _Pattern Pattern;

struct _Pattern
{
    VibeUInt8   *data;
    gint        pattern_id;
};

struct _NgfVibrator
{
    VibeInt32   device;
    GHashTable  *vibrator_data;
    GHashTable  *patterns;
};

NgfVibrator*
ngf_vibrator_create ()
{
    NgfVibrator *self = NULL;

    if ((self = g_new0 (NgfVibrator, 1)) == NULL)
        goto failed;

    if (VIBE_FAILED (ImmVibeInitialize (VIBE_CURRENT_VERSION_NUMBER)))
        goto failed;

    if (VIBE_FAILED (ImmVibeOpenDevice (0, &self->device)))
        goto failed;

    if ((self->patterns = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free)) == NULL)
        goto failed;

    if ((self->vibrator_data = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free)) == NULL)
        goto failed;

    return self;

failed:
    ngf_vibrator_destroy (self);
    return NULL;
}

void
ngf_vibrator_destroy (NgfVibrator *self)
{
    if (self == NULL)
        return;

    if (self->device != VIBE_INVALID_DEVICE_HANDLE_VALUE) {
        ImmVibeStopAllPlayingEffects (self->device);
        ImmVibeCloseDevice (self->device);
        self->device = VIBE_INVALID_DEVICE_HANDLE_VALUE;
        ImmVibeTerminate ();
    }

    if (self->vibrator_data) {
        g_hash_table_destroy (self->vibrator_data);
        self->vibrator_data = NULL;
    }

    if (self->patterns) {
        g_hash_table_destroy (self->patterns);
        self->patterns = NULL;
    }

    g_free (self);
}

VibeUInt8*
_load_ivt (const char *filename)
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

        return data;
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

gboolean
ngf_vibrator_register (NgfVibrator *self, const char *name, const char *filename, gint pattern_id)
{
    Pattern *p = NULL;
    VibeUInt8 *data = NULL;

    if (self ==  NULL || name == NULL || pattern_id < 0)
        return FALSE;

    /* Lookup for the pattern, if we have already registered one with the similar name then
       we will ignore this one. */

    if ((p = (Pattern*) g_hash_table_lookup (self->patterns, name)) != NULL)
        return FALSE;

    /* Lookup for the given IVT filename to see if it has been loaded. If not, let's load
       it. */

    if (filename) {
        if ((data = (VibeUInt8*) g_hash_table_lookup (self->vibrator_data, filename)) == NULL) {
            if ((data = _load_ivt (filename)) == NULL)
                return FALSE;

            g_hash_table_replace (self->vibrator_data, g_strdup (filename), data);
        }
    }

    /* We don't have entry with the given name, so we will make one. */

    if ((p = g_new0 (Pattern, 1)) == NULL)
        return FALSE;

    p->data         = data;
    p->pattern_id   = pattern_id;

    g_hash_table_replace (self->patterns, g_strdup (name), p);

    return TRUE;
}

guint
ngf_vibrator_start_file (NgfVibrator *self, const char *filename, gint pattern)
{
    gint id = 0;
    VibeUInt8 *effects = NULL;

    if (self == NULL)
        return 0;
    
    effects=_load_ivt (filename);
    if (!effects)
        return 0;
    
    ImmVibePlayIVTEffect (self->device, effects, pattern, &id);
    
    g_free (effects);
    
    return id;
}

guint
ngf_vibrator_start (NgfVibrator *self, const char *name)
{
    gint id = 0;
    VibeUInt8 *effects = NULL;
    Pattern *p = NULL;

    if (self == NULL)
        return 0;

    if ((p = (Pattern*) g_hash_table_lookup (self->patterns, name)) == NULL)
        return 0;

    if (p->data)
        effects = p->data;
    else
        effects = g_pVibeIVTBuiltInEffects;

    ImmVibePlayIVTEffect (self->device, effects, p->pattern_id, &id);
    
    return id;
}

void
ngf_vibrator_stop (NgfVibrator *self, gint id)
{
    VibeStatus status;
    VibeInt32 effect_state = 0;

    if (self == NULL || id < 0)
        return;

    status = ImmVibeGetEffectState (self->device, id, &effect_state);
    if (VIBE_SUCCEEDED (status)) {
        ImmVibeStopPlayingEffect (self->device, id);
    }
}

gboolean
ngf_vibrator_is_completed (NgfVibrator *self, gint id)
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
ngf_vibrator_is_repeating (NgfVibrator *self, const char *name)
{
    VibeUInt8 *effects = NULL;
    Pattern *p = NULL;
    VibeInt32 duration = 0;

    if (self == NULL)
        return FALSE;

    if ((p = (Pattern*) g_hash_table_lookup (self->patterns, name)) == NULL)
        return FALSE;

    if (p->data)
        effects = p->data;
    else
        effects = g_pVibeIVTBuiltInEffects;
    
    ImmVibeGetIVTEffectDuration (effects, p->pattern_id, &duration);
    if (duration == VIBE_TIME_INFINITE)
        return TRUE;
    
    return FALSE;
}
