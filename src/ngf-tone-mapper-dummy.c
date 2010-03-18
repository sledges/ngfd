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

#include "ngf-tone-mapper.h"

struct _NgfToneMapper
{
    int dummy;
};

NgfToneMapper*
ngf_tone_mapper_create ()
{
    NgfToneMapper *self = NULL;

    if ((self = g_new0 (NgfToneMapper, 1)) == NULL) {
        goto failed;
    }

    return self;

failed:
    ngf_tone_mapper_destroy (self);
    return NULL;
}

void
ngf_tone_mapper_destroy (NgfToneMapper *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

const char*
ngf_tone_mapper_get_tone (NgfToneMapper *self, const char *uri)
{
    (void) self;
    (void) uri;

    return NULL;
}
