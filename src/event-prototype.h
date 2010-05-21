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

#ifndef EVENT_PROTOTYPE_H
#define EVENT_PROTOTYPE_H

#include <glib.h>
#include <pulse/proplist.h>

typedef struct _EventPrototype EventPrototype;

struct _EventPrototype
{
    gchar       **allowed_keys;         /* NULL terminated array of keys that user can override. */
    GHashTable   *properties;           /* Collection of properties. */
    pa_proplist  *stream_properties;    /* Stream properties. */
};

EventPrototype* event_prototype_new   ();
EventPrototype* event_prototype_copy  (EventPrototype *source);
void            event_prototype_merge (EventPrototype *target, EventPrototype *source);
void            event_prototype_free  (EventPrototype *proto);
void            event_prototype_dump  (EventPrototype *proto);

#endif /* EVENT_PROTOTYPE_H */
