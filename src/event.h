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

#ifndef EVENT_H
#define EVENT_H

#include <glib.h>
#include "context.h"
#include "controller.h"
#include "audio-stream.h"
#include "event-prototype.h"

/* Resource flags */
#define RESOURCE_AUDIO      (1 << 1)
#define RESOURCE_VIBRATION  (1 << 2)
#define RESOURCE_LED        (1 << 3)
#define RESOURCE_BACKLIGHT  (1 << 4)

/* Play mode flags */
#define PLAY_MODE_LONG      (1 << 1)
#define PLAY_MODE_SHORT     (1 << 2)

typedef enum    _EventState  EventState;
typedef struct  _Event       Event;
typedef void    (*EventCallback) (Event *event, EventState state, gpointer userdata);

enum _EventState
{
    EVENT_NONE = 0,
    EVENT_STARTED,
    EVENT_COMPLETED,
    EVENT_FAILED
};

struct _Event
{
    EventPrototype      *proto;
    GHashTable          *properties;

    guint               policy_id;
    gint                resources;
    gint                play_mode;
    guint               play_timeout;

    /* Internal event specific data */
    guint               max_length_timeout_id;

    AudioStream         *audio_stream;
    Controller          *audio_volume_controller;
    guint               audio_volume_id;
    gboolean            audio_volume_set;
    gboolean            audio_use_fallback;
    const gchar         *audio_filename;

    gboolean            audio_repeat_enabled;
    gint                audio_repeat_count;
    gint                audio_max_repeats;

    guint               vibra_id;
    guint               tone_generator_id;
    guint               led_id;
    guint               vibra_poll_id;

    gboolean            audio_ready;
    gpointer            vibra_data;

    /* Startup timer for monitoring event length */
    GTimer              *start_timer;

    /* Context containing the backends */
    Context             *context;

    /* User supplied callback and userdata */
    InterfaceReadyCallback  iface_callback;
    EventCallback       callback;
    gpointer            userdata;
};

Event*   event_new (Context *context, EventPrototype *proto);
void        event_free (Event *self);

void        event_set_callback (Event *self, EventCallback callback, gpointer userdata);

gboolean    event_start (Event *self, GHashTable *properties);
void        event_stop (Event *self);

#endif /* EVENT_H */
