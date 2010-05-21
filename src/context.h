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

#include "event-definition.h"
#include "event-prototype.h"
#include "event.h"

#include "profile.h"
#include "tone-mapper.h"
#include "audio.h"
#include "vibrator.h"
#include "tone-generator.h"
#include "led.h"
#include "backlight.h"
#include "dbus-if.h"

struct _Context
{
    GMainLoop     *loop;
    DBusIf        *dbus;

    GHashTable    *definitions;
    GHashTable    *prototypes;
    GList         *event_list;

    Profile       *profile;
    ToneMapper    *tone_mapper;
    Audio         *audio;
    Vibrator      *vibrator;
    ToneGenerator *tonegen;
    Led           *led;
    Backlight     *backlight;
};

#endif /* CONTEXT_H */
