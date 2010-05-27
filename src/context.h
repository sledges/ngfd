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

struct _Context
{
    GMainLoop     *loop;

    GHashTable    *definitions;
    GHashTable    *events;
    GList         *request_list;

    ToneMapper    *tone_mapper;
    Audio         *audio;
    Vibrator      *vibrator;

    DBusConnection    *system_bus;      /* system bus */
    DBusConnection    *session_bus;     /* session bus */

    SoundPath        **sounds;          /* all sound defined in the configuration, NULL terminated */
    guint              num_sounds;

    Volume           **volumes;         /* all volumes defined in the configuration, NULL terminated */
    guint              num_volumes;

    VibrationPattern **patterns;        /* NULL terminated array of all vibration patterns defined in the configuration */  
    guint              num_patterns;

    gchar             *active_profile;
    gboolean           silent_mode;
    gboolean           meeting_mode;
    gboolean           vibration_enabled;
};

SoundPath*        context_add_sound_path (Context *context, SoundPath *sound_path);
Volume*           context_add_volume     (Context *context, Volume *volume);
VibrationPattern* context_add_pattern    (Context *context, VibrationPattern *pattern);

#endif /* CONTEXT_H */
