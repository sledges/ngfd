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
#include "event-prototype.h"

EventPrototype*
event_prototype_new ()
{
    EventPrototype *proto = NULL;

    if ((proto = g_new0 (EventPrototype, 1)) == NULL)
        return NULL;

    if ((proto->properties = properties_new ()) == NULL) {
        g_free (proto);
        return NULL;
    }

    if ((proto->stream_properties = pa_proplist_new ()) == NULL) {
        g_hash_table_destroy (proto->properties);
        g_free (proto);
        return NULL;
    }

    return proto;
}

EventPrototype*
event_prototype_copy (EventPrototype *source)
{
    EventPrototype *p = NULL;

    p = event_prototype_new ();
    p->properties = properties_copy (source->properties);
    p->stream_properties = pa_proplist_copy (source->stream_properties);
    return p;
}

void
event_prototype_merge (EventPrototype *target, EventPrototype *source)
{
    properties_merge (target->properties, source->properties);
    pa_proplist_update (target->stream_properties, PA_UPDATE_REPLACE, source->stream_properties);
}

void
event_prototype_free (EventPrototype *proto)
{
    if (proto == NULL)
        return;

    if (proto->allowed_keys) {
        g_strfreev (proto->allowed_keys);
        proto->allowed_keys = NULL;
    }

    if (proto->properties) {
        g_hash_table_destroy (proto->properties);
        proto->properties = NULL;
    }

    if (proto->stream_properties) {
        pa_proplist_free (proto->stream_properties);
        proto->stream_properties = NULL;
    }

    g_free (proto);
}

void
event_prototype_dump (EventPrototype *proto)
{
    properties_dump (proto->properties);
}
