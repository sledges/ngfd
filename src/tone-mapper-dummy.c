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

#include "tone-mapper.h"

struct _ToneMapper
{
    int dummy;
};

ToneMapper*
tone_mapper_create ()
{
    ToneMapper *self = NULL;

    if ((self = g_new0 (ToneMapper, 1)) == NULL) {
        goto failed;
    }

    return self;

failed:
    tone_mapper_destroy (self);
    return NULL;
}

void
tone_mapper_destroy (ToneMapper *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

const char*
tone_mapper_get_tone (ToneMapper *self, const char *uri)
{
    (void) self;
    (void) uri;

    return NULL;
}
