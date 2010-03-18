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

#ifndef NGF_EVENT_H
#define NGF_EVENT_H

#include <glib.h>
#include "ngf-context.h"
#include "ngf-audio-stream.h"
#include "ngf-event-prototype.h"

/* Resource flags */
#define NGF_RESOURCE_AUDIO      (1 << 1)
#define NGF_RESOURCE_VIBRATION  (1 << 2)
#define NGF_RESOURCE_LED        (1 << 3)
#define NGF_RESOURCE_BACKLIGHT  (1 << 4)

/* Play mode flags */
#define NGF_PLAY_MODE_LONG      (1 << 1)
#define NGF_PLAY_MODE_SHORT     (1 << 2)

typedef enum    _NgfEventState  NgfEventState;
typedef struct  _NgfEvent       NgfEvent;
typedef void    (*NgfEventCallback) (NgfEvent *event, NgfEventState state, gpointer userdata);

enum _NgfEventState
{
    NGF_EVENT_STARTED,
    NGF_EVENT_COMPLETED,
    NGF_EVENT_FAILED
};

struct _NgfEvent
{
    NgfEventPrototype   *proto;
    GHashTable          *properties;

    guint               policy_id;
    gint                resources;
    gint                play_mode;
    guint               play_timeout;

    /* Internal event specific data */
    guint               max_length_timeout_id;

    NgfAudioStream     *audio_stream;
    guint               audio_volume_id;
    gboolean            audio_volume_set;
    gboolean            audio_use_fallback;
    gint                audio_repeat_count;

    guint               vibra_id;
    guint               tonegen_id;
    guint               led_id;
    guint               backlight_id;

    /* Startup timer for monitoring event length */
    GTimer              *start_timer;

    /* Context containing the backends */
    NgfContext          *context;

    /* User supplied callback and userdata */
    NgfEventCallback    callback;
    gpointer            userdata;
};

NgfEvent*   ngf_event_new (NgfContext *context, NgfEventPrototype *proto);
void        ngf_event_free (NgfEvent *self);

void        ngf_event_set_callback (NgfEvent *self, NgfEventCallback callback, gpointer userdata);

gboolean    ngf_event_start (NgfEvent *self, GHashTable *properties);
void        ngf_event_stop (NgfEvent *self);

#endif /* NGF_EVENT_H */
