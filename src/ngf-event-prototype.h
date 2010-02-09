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

#ifndef NGF_EVENT_PROTOTYPE_H
#define NGF_EVENT_PROTOTYPE_H

#include <glib.h>
#include <pulse/proplist.h>
#include "ngf-value.h"

typedef struct _NgfEventPrototype NgfEventPrototype;

struct _NgfEventPrototype
{
    gint        max_length;

    gchar       *tone_filename;
    gchar       *tone_key;
    gchar       *tone_profile;

    gchar       *volume_key;
    gchar       *volume_profile;

    /* Stream properties */
    pa_proplist *stream_properties;
};

NgfEventPrototype*  ngf_event_prototype_new ();
void                ngf_event_prototype_free (NgfEventPrototype *proto);
void                ngf_event_prototype_dump (NgfEventPrototype *proto);

#endif /* NGF_EVENT_PROTOTYPE_H */
