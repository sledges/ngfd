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

#include "log.h"
#include "properties.h"
#include "event.h"

Event*
event_new ()
{
    Event *event = NULL;

    if ((event = g_new0 (Event, 1)) == NULL)
        return NULL;

    if ((event->properties = properties_new ()) == NULL) {
        g_free (event);
        return NULL;
    }

    return event;
}

void
event_free (Event *event)
{
    if (event == NULL)
        return;

    if (event->sounds) {
        g_list_free (event->sounds);
        event->sounds = NULL;
    }

    if (event->patterns) {
        g_list_free (event->patterns);
        event->patterns = NULL;
    }

    g_free (event->event_id);
    g_free (event->led_pattern);
    g_free (event);
}

void
event_dump (Event *event)
{
    properties_dump (event->properties);
}
