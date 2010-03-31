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

#include "ngf-log.h"
#include "ngf-properties.h"
#include "ngf-event-prototype.h"

NgfEventPrototype*
ngf_event_prototype_new ()
{
    NgfEventPrototype *proto = NULL;

    if ((proto = g_new0 (NgfEventPrototype, 1)) == NULL)
        return NULL;

    if ((proto->properties = ngf_properties_new ()) == NULL) {
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

NgfEventPrototype*
ngf_event_prototype_copy (NgfEventPrototype *source)
{
    NgfEventPrototype *p = NULL;

    p = ngf_event_prototype_new ();
    p->properties = ngf_properties_copy (source->properties);
    p->stream_properties = pa_proplist_copy (source->stream_properties);
    return p;
}

void
ngf_event_prototype_merge (NgfEventPrototype *target, NgfEventPrototype *source)
{
    ngf_properties_merge (target->properties, source->properties);
    pa_proplist_update (target->stream_properties, PA_UPDATE_REPLACE, source->stream_properties);
}

void
ngf_event_prototype_free (NgfEventPrototype *proto)
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
ngf_event_prototype_dump (NgfEventPrototype *proto)
{
    ngf_properties_dump (proto->properties);
}
