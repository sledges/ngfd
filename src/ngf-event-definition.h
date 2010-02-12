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

#ifndef NGF_EVENT_DEF_H
#define NGF_EVENT_DEF_H

#include <glib.h>

typedef struct _NgfEventDefinition NgfEventDefinition;

struct _NgfEventDefinition
{
    gchar   *long_proto;
    gchar   *short_proto;
};

NgfEventDefinition* ngf_event_definition_new ();
void                ngf_event_definition_free (NgfEventDefinition *self);

#endif /* NGF_EVENT_DEF */
