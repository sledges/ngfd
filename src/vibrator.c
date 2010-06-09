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

#include "log.h"
#include "vibrator.h"

#define POLL_TIMEOUT 500

typedef struct _Pattern Pattern;

struct _Pattern
{
    guint            id;            /* immersion vibration event id */
    guint            pattern_id;    /* pattern id within effect data */
    gpointer         data;          /* effect data */
    guint            poll_id;       /* source id for poll */
    Vibrator        *vibrator;

    VibratorCompletedCallback callback;
    gpointer                  userdata;
};

struct _Vibrator
{
    VibeInt32  device;          /* immersion device id */
    GList     *patterns;        /* list of active patterns */
};

static gboolean pattern_poll_cb      (gpointer userdata);
static Pattern* pattern_new          (Vibrator *vibrator, guint id, gpointer data, guint pattern_id, VibratorCompletedCallback callback, gpointer userdata);
static void     pattern_free         (Pattern *p);
static Pattern* pattern_lookup       (Vibrator *vibrator, guint id);
static gboolean pattern_is_completed (Vibrator *vibrator, gint id);
static gboolean pattern_is_repeating (gpointer data, gint pattern_id);



static gboolean
pattern_poll_cb (gpointer userdata)
{
    Pattern  *p        = (Pattern*) userdata;
    Vibrator *vibrator = p->vibrator;

    if (pattern_is_completed (vibrator, p->id)) {
        LOG_DEBUG ("%s >> vibration has been completed.", __FUNCTION__);

        p->poll_id = 0;
        if (p->callback)
            p->callback (vibrator, p->userdata);

        return FALSE;
    }

    return TRUE;
}

static Pattern*
pattern_new (Vibrator *vibrator, guint id, gpointer data, guint pattern_id, VibratorCompletedCallback callback, gpointer userdata)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Pattern *p = NULL;

    p = g_slice_new0 (Pattern);
    p->id         = id;
    p->pattern_id = pattern_id;
    p->data       = data;
    p->callback   = callback;
    p->userdata   = userdata;
    p->vibrator   = vibrator;

    if (!pattern_is_repeating (p->data, p->pattern_id)) {
        LOG_DEBUG ("%s >> pattern is finite, poll for completion.", __FUNCTION__);
        p->poll_id = g_timeout_add (POLL_TIMEOUT, pattern_poll_cb, p);
    }

    return p;
}

static void
pattern_free (Pattern *p)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    if (p->poll_id > 0) {
        g_source_remove (p->poll_id);
        p->poll_id = 0;
    }

    g_slice_free (Pattern, p);
}

static Pattern*
pattern_lookup (Vibrator *vibrator, guint id)
{
    LOG_DEBUG ("%s >> entering (id %u)", __FUNCTION__, id);

    GList   *iter = NULL;
    Pattern *p    = NULL;

    for (iter = g_list_first (vibrator->patterns); iter; iter = g_list_next (iter)) {
        p = (Pattern*) iter->data;
        if (p->id == id) {
            return p;
        }
    }

    return NULL;
}

Vibrator*
vibrator_create ()
{
    Vibrator *vibrator = NULL;

    if ((vibrator = g_new0 (Vibrator, 1)) == NULL)
        goto failed;

    if (VIBE_FAILED (ImmVibeInitialize (VIBE_CURRENT_VERSION_NUMBER)))
        goto failed;

    if (VIBE_FAILED (ImmVibeOpenDevice (0, &vibrator->device)))
        goto failed;

    return vibrator;

failed:
    vibrator_destroy (vibrator);
    return NULL;
}

void
vibrator_destroy (Vibrator *vibrator)
{
    if (vibrator == NULL)
        return;

    if (vibrator->device != VIBE_INVALID_DEVICE_HANDLE_VALUE) {
        ImmVibeStopAllPlayingEffects (vibrator->device);
        ImmVibeCloseDevice (vibrator->device);
        vibrator->device = VIBE_INVALID_DEVICE_HANDLE_VALUE;
        ImmVibeTerminate ();
    }

    g_free (vibrator);
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
vibrator_start (Vibrator *vibrator, gpointer data, gint pattern_id, VibratorCompletedCallback callback, gpointer userdata)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    VibeUInt8 *effects = data ? (VibeUInt8*) data : g_pVibeIVTBuiltInEffects;
    gint       id      = 0;
    Pattern   *p       = NULL;

    if (vibrator == NULL)
        return 0;

    if (VIBE_SUCCEEDED (ImmVibePlayIVTEffect (vibrator->device, effects, pattern_id, &id))) {
        p = pattern_new (vibrator, id, effects, pattern_id, callback, userdata);
        vibrator->patterns = g_list_append (vibrator->patterns, p);
        LOG_DEBUG ("%s >> started pattern with id %d", __FUNCTION__, p->id);
        return p->id;
    }

    return 0;
}

void
vibrator_stop (Vibrator *vibrator, guint id)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Pattern *p = NULL;

    if (vibrator == NULL || id == 0)
        return;

    if ((p = pattern_lookup (vibrator, id))) {
        LOG_DEBUG ("%s >> stopping effect %d", __FUNCTION__, id);
        ImmVibeStopPlayingEffect (vibrator->device, id);
        vibrator->patterns = g_list_remove (vibrator->patterns, p);
        pattern_free (p);
    }
}

static gboolean
pattern_is_completed (Vibrator *vibrator, gint id)
{
    VibeStatus status;
    VibeInt32 effect_state = 0;

    status = ImmVibeGetEffectState (vibrator->device, id, &effect_state);
    if (VIBE_SUCCEEDED (status)) {
        if (status != VIBE_EFFECT_STATE_PLAYING)
            return FALSE;
    }

    return TRUE;
}

static gboolean
pattern_is_repeating (gpointer data, gint pattern_id)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    VibeInt32 duration = 0;

    if (VIBE_SUCCEEDED (ImmVibeGetIVTEffectDuration ((VibeUInt8*) data, pattern_id, &duration))) {
        if (duration == VIBE_TIME_INFINITE)
            return TRUE;
    }
    else
        LOG_WARNING ("%s >> failed to query pattern duration", __FUNCTION__);

    return FALSE;
}
