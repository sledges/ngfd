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

    if ((event->stream_properties = pa_proplist_new ()) == NULL) {
        g_hash_table_destroy (event->properties);
        g_free (event);
        return NULL;
    }

    return event;
}

Event*
event_copy (Event *source)
{
    Event *event = NULL;

    event = event_new ();
    event->properties = properties_copy (source->properties);
    event->stream_properties = pa_proplist_copy (source->stream_properties);
    return event;
}

void
event_merge (Event *target, Event *source)
{
    properties_merge (target->properties, source->properties);
    pa_proplist_update (target->stream_properties, PA_UPDATE_REPLACE, source->stream_properties);
}

void
event_free (Event *event)
{
    if (event == NULL)
        return;

    if (event->allowed_keys) {
        g_strfreev (event->allowed_keys);
        event->allowed_keys = NULL;
    }

    if (event->properties) {
        g_hash_table_destroy (event->properties);
        event->properties = NULL;
    }

    if (event->stream_properties) {
        pa_proplist_free (event->stream_properties);
        event->stream_properties = NULL;
    }

    g_free (event);
}

void
event_dump (Event *event)
{
    properties_dump (event->properties);
}
