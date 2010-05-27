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

#include <stdlib.h>
#include <profiled/libprofile.h>

#include "sound-path.h"
#include "volume.h"
#include "profile.h"

#define KEY_VIBRATION_ENABLED "vibrating.alert.enabled"
#define SILENT_PROFILE        "silent"
#define MEETING_PROFILE       "meeting"

#define TONE_SUFFIX           ".tone"
#define VOLUME_SUFFIX         ".volume"

static void
resolve_sound_path (Context    *context,
                    const char *profile,
                    const char *key,
                    const char *value)
{
    SoundPath **i = NULL;
    SoundPath  *s = NULL;

    if (context->active_profile == NULL)
        return;

    if (!g_str_has_suffix (key, TONE_SUFFIX))
        return;

    for (i = context->sounds; *i; ++i) {
        s = (SoundPath*) (*i);

        if (s->type != SOUND_PATH_TYPE_PROFILE)
            continue;

        if (!g_str_equal (s->key, key))
            continue;

        if ((!s->profile && g_str_equal (context->active_profile, profile)) || (s->profile && g_str_equal (s->profile, profile))) {
            g_print ("+ filename changed %s -> %s\n", s->filename, value);
            g_free (s->filename);
            s->filename = g_strdup (value);
            g_print ("sound_path <type=%d, filename=%s, key=%s, profile=%s\n", s->type, s->filename, s->key, s->profile);
            break;
        }
    }
}

static void
resolve_volume (Context    *context,
                const char *profile,
                const char *key,
                const char *value)
{
    Volume **i = NULL;
    Volume  *s = NULL;

    if (context->active_profile == NULL)
        return;

    if (!g_str_has_suffix (key, VOLUME_SUFFIX))
        return;

    for (i = context->volumes; *i; ++i) {
        s = (Volume*) (*i);

        if (s->type != VOLUME_TYPE_PROFILE)
            continue;

        if (!g_str_equal (s->key, key))
            continue;

        if ((!s->profile && g_str_equal (context->active_profile, profile)) || (s->profile && g_str_equal (s->profile, profile))) {
            g_print ("+ volume changed %d -> %d\n", s->level, profile_parse_int (value));
            s->level = profile_parse_int (value);
            g_print ("volume <type=%d, level=%d, key=%s, profile=%s\n",
                s->type, s->level, s->key, s->profile);
            break;
        }
    }
}

static void
resolve_profile (Context    *context,
                 const char *profile)
{
    context->silent_mode  = FALSE;
    context->meeting_mode = FALSE;

    if (g_str_equal (profile, SILENT_PROFILE))
        context->silent_mode = TRUE;

    else if (g_str_equal (profile, MEETING_PROFILE))
        context->meeting_mode = TRUE;

    g_free (context->active_profile);
    context->active_profile = g_strdup (profile);
}

static void
value_changed_cb (const char *profile,
				  const char *key,
				  const char *value,
				  const char *type,
				  void *userdata)
{
    Context *context = (Context*) userdata;

    if (context->active_profile && g_str_equal (context->active_profile, profile)) {
        if (g_str_equal (key, KEY_VIBRATION_ENABLED)) {
            context->vibration_enabled = profile_parse_bool (value);
            return;
        }
    }

    resolve_sound_path (context, profile, key, value);
    resolve_volume     (context, profile, key, value);
}

static void
profile_changed_cb (const char *profile,
                    void *userdata)
{
    Context *context = (Context*) userdata;
    g_print ("profile changed %s -> %s\n", context->active_profile, profile);
    resolve_profile (context, profile);
}

int
profile_create (Context *context)
{
    profile_track_add_active_cb  (value_changed_cb, context, NULL);
    profile_track_add_change_cb  (value_changed_cb, context, NULL);
    profile_track_add_profile_cb (profile_changed_cb, context, NULL);

    profile_tracker_init ();

    return TRUE;
}

int
profile_resolve (Context *context)
{
    SoundPath **i = NULL;
    Volume    **j = NULL;

    context->active_profile    = profile_get_profile ();
    context->vibration_enabled = profile_get_value_as_bool (NULL, KEY_VIBRATION_ENABLED);

    if (g_str_equal (context->active_profile, SILENT_PROFILE))
        context->silent_mode = TRUE;
    else if (g_str_equal (context->active_profile, MEETING_PROFILE))
        context->meeting_mode = TRUE;


    for (i = context->sounds; *i; ++i) {
        SoundPath *s = (SoundPath*) (*i);

        if (s->type != SOUND_PATH_TYPE_PROFILE)
            continue;

        g_free (s->filename);
        s->filename = g_strdup (profile_get_value (s->profile, s->key));
        g_print ("sound_path <type=%d, filename=%s, key=%s, profile=%s\n", s->type, s->filename, s->key, s->profile);
    }

    for (j = context->volumes; *j; ++j) {
        Volume *v = (Volume*) (*j);

        if (v->type != VOLUME_TYPE_PROFILE)
            continue;

        v->level = profile_get_value_as_int (v->profile, v->key);
        g_print ("volume <type=%d, level=%d, key=%s, profile=%s\n", v->type, v->level, v->key, v->profile);
    }

    return TRUE;
}

void
profile_destroy (Context *context)
{
    (void) context;
    profile_tracker_quit ();
}


