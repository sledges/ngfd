/*
 * ngfd - Non-graphic feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation.
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

    gboolean          backlight_enabled;        /* keep the backlight alive */
};

Event* event_new   ();
void   event_free  (Event *event);

#endif /* EVENT_H */
