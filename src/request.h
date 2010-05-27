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

#ifndef REQUEST_H
#define REQUEST_H

#include <glib.h>

#include "context.h"
#include "audio-stream.h"
#include "resources.h"
#include "sound-path.h"
#include "vibration-pattern.h"
#include "event.h"

enum
{
    REQUEST_PLAY_MODE_LONG = 1,
    REQUEST_PLAY_MODE_SHORT
};

enum
{
    REQUEST_STATE_NONE = 0,
    REQUEST_STATE_STARTED,
    REQUEST_STATE_COMPLETED,
    REQUEST_STATE_UPDATED,
    REQUEST_STATE_FAILED
};

typedef struct  _Request Request;

struct _Request
{
    /* user */

    gchar            *name;                         /* request name */
    guint             policy_id;                    /* policy identifier */
    gint              resources;                    /* bitmask of resources @see resources.h */
    gint              play_mode;                    /* REQUEST_PLAY_MODE_LONG or REQUEST_PLAY_MODE_SHORT */
    guint             play_timeout;                 /* play timeout in seconds */

    SoundPath        *custom_sound;                 /* custom sound */

    void              (*callback) (Request *request, guint state, gpointer userdata);
    gpointer          userdata;

    /* internal */

    Context          *context;
    Event            *event;                        /* event the request is based on */

    guint             max_timeout_id;               /* maximum timeout source id */
    guint             controller_id;                /* volume controller id */
    gint              repeat_count;                 /* number of times audio has been repeated */

    SoundPath        *active_sound;                 /* currently active sound path */
    GList            *sound_iterator;               /* position in the sound list */
    AudioStream      *stream;                       /* audio playback stream */

    VibrationPattern *active_pattern;               /* currently active vibration pattern */
    VibrationPattern *custom_pattern;               /* custom vibration pattern */
    GList            *vibration_iterator;           /* position in the vibration list */
    guint             vibration_id;                 /* vibration play id */

    gboolean          synchronize_done;             /* synchronization has been done */
    gboolean          tone_generator_active;        /* tone generator has been activated */
    gboolean          leds_active;
    gboolean          backlight_active;
};

Request* request_new              (Context *context, Event *event);
void     request_free             (Request *request);
void     request_set_custom_sound (Request *request, const char *path);

#endif /* REQUEST_H */
