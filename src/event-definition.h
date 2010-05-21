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

#ifndef EVENT_DEF_H
#define EVENT_DEF_H

#include <glib.h>

typedef struct _EventDefinition EventDefinition;

struct _EventDefinition
{
    gchar   *long_proto;
    gchar   *short_proto;
    gchar   *meeting_proto;
};

EventDefinition* event_definition_new  ();
void             event_definition_free (EventDefinition *self);

#endif /* EVENT_DEF */
