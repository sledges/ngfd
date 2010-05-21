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

#ifndef EVENT_H
#define EVENT_H

#include <glib.h>
#include <pulse/proplist.h>

typedef struct _Event Event;

struct _Event
{
    gchar       **allowed_keys;         /* NULL terminated array of keys that user can override. */
    GHashTable   *properties;           /* Collection of properties. */
    pa_proplist  *stream_properties;    /* Stream properties. */
};

Event* event_new   ();
Event* event_copy  (Event *source);
void   event_merge (Event *target, Event *source);
void   event_free  (Event *event);
void   event_dump  (Event *event);

#endif /* EVENT_H */
