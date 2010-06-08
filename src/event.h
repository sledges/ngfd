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
#include <pulse/proplist.h>

#include "sound-path.h"
#include "vibration-pattern.h"
#include "volume.h"

typedef struct _Event Event;

struct _Event
{
    GHashTable       *properties;               /* Collection of properties. */

    gboolean          allow_custom;             /* allow custom sound */
    gint              max_timeout;              /* maximum timeout for event */

    GList            *sounds;                   /* sounds defined for the event. collection of pointers to context->sounds */
    Volume           *volume;                   /* volume for the event, pointer to context->volumes */
    gboolean          silent_enabled;           /* play in silent mode */
    gboolean          repeat;                   /* repeat sound */
    gint              num_repeats;              /* number of times to repeat or 0 for infinite */
    gchar            *event_id;                 /* event id for stream */

    gboolean          lookup_pattern;           /* lookup a custom vibration pattern */
    GList            *patterns;                 /* vibration patterns in the order of importance */

    gint              tone_generator_pattern;   /* tone generator pattern */
    gchar            *led_pattern;

    gboolean          tone_generator_enabled;
    gboolean          audio_enabled;
    gboolean          vibration_enabled;
    gboolean          leds_enabled;

    gboolean          unlock_tklock;            /* unlock touchscreen and keyboard lock */
    gboolean          backlight_enabled;        /* keep the backlight alive */
};

Event* event_new   ();
void   event_free  (Event *event);

#endif /* EVENT_H */
