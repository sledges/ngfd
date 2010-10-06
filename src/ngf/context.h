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

#ifndef CONTEXT_H
#define CONTEXT_H

#include <glib.h>

#include <glib.h>
#include <dbus/dbus.h>

typedef struct _Context Context;

#include "definition.h"
#include "event.h"
#include "request.h"

#include "tone-mapper.h"
#include "audio.h"
#include "vibrator.h"

#include "sound-path.h"
#include "vibration-pattern.h"
#include "volume.h"

#include "core-internal.h"

struct _Context
{
    NCore             *core;                /* core plugin loading functionality */
    GList             *required_plugins;

    GMainLoop         *loop;

    GHashTable        *definitions;
    GHashTable        *events;
    GList             *request_list;

    Audio             *audio;
    Vibrator          *vibrator;

    gint               audio_buffer_time;   /* buffer time */
    gint               audio_latency_time;  /* backend latency time */

    DBusConnection    *system_bus;          /* system bus */
    DBusConnection    *session_bus;         /* session bus */

    SoundPath        **sounds;              /* all sound defined in the configuration, NULL terminated */
    guint              num_sounds;

    Volume           **volumes;             /* all volumes defined in the configuration, NULL terminated */
    guint              num_volumes;
    guint              system_volume[3];

    VibrationPattern **patterns;            /* NULL terminated array of all vibration patterns defined in the configuration */
    guint              num_patterns;
    gchar             *patterns_path;
    gchar             *sound_path;

    gchar             *active_profile;
    gboolean           silent_mode;
    gboolean           meeting_mode;
    gboolean           vibration_enabled;

    gchar             *mapped_tone_path;
    GHashTable        *mapped_tones;

    DBusConnection    *volume_bus;          /* DBus connection to Pulseaudio */
    guint              volume_retry_id;     /* source id for retry connection */
    GQueue            *volume_queue;        /* queued volume ops while Pulseaudio DBus connection is down */
};

SoundPath*        context_add_sound_path (Context *context, SoundPath *sound_path);
Volume*           context_add_volume     (Context *context, Volume *volume);
VibrationPattern* context_add_pattern    (Context *context, VibrationPattern *pattern);

#endif /* CONTEXT_H */
