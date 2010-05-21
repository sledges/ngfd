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

enum
{
    REQUEST_STATE_NONE = 0,
    REQUEST_STATE_STARTED,
    REQUEST_STATE_COMPLETED,
    REQUEST_STATE_FAILED
};

typedef struct  _Request Request;
typedef void (*RequestCallback) (Request *request, guint state, gpointer userdata);

struct _Request
{
    EventPrototype *proto;
    GHashTable     *properties;

    guint           policy_id;
    gint            resources;
    gint            play_mode;
    guint           play_timeout;

    guint           max_length_timeout_id;

    AudioStream    *audio_stream;
    Controller     *audio_volume_controller;
    guint           audio_volume_id;
    gboolean        audio_volume_set;
    gboolean        audio_use_fallback;
    const gchar    *audio_filename;

    gboolean        audio_repeat_enabled;
    gint            audio_repeat_count;
    gint            audio_max_repeats;

    guint           vibra_id;
    guint           tone_generator_id;
    guint           led_id;
    guint           vibra_poll_id;

    gboolean        audio_ready;
    gpointer        vibra_data;

    GTimer         *start_timer;
    Context        *context;

    InterfaceReadyCallback iface_callback;
    RequestCallback        callback;
    gpointer               userdata;
};

Request* request_new          (Context *context, EventPrototype *proto);
void     request_free         (Request *request);
void     request_set_callback (Request *request, RequestCallback callback, gpointer userdata);
gboolean request_start        (Request *request, GHashTable *properties);
void     request_stop         (Request *request);

#endif /* REQUEST_H */
