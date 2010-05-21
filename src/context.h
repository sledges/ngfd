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

#include "profile.h"
#include "tone-mapper.h"
#include "audio.h"
#include "vibrator.h"

struct _Context
{
    GMainLoop     *loop;

    GHashTable    *definitions;
    GHashTable    *events;
    GList         *request_list;

    Profile       *profile;
    ToneMapper    *tone_mapper;
    Audio         *audio;
    Vibrator      *vibrator;

    DBusConnection *system_bus;     /* system bus */
    DBusConnection *session_bus;    /* session bus */
};

#endif /* CONTEXT_H */
