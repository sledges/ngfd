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

#include <stdio.h>

#include "ngf-vibrator.h"

struct _NgfVibrator
{
    int dummy;
};

NgfVibrator*
ngf_vibrator_create ()
{
    NgfVibrator *self = NULL;

    if ((self = g_new0 (NgfVibrator, 1)) == NULL)
        return NULL;

    return self;
}

void
ngf_vibrator_destroy (NgfVibrator *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

gboolean
ngf_vibrator_register (NgfVibrator *self, const char *name, const char *filename, gint pattern_id)
{
    (void) self;
    (void) name;
    (void) filename;
    (void) pattern_id;

    return TRUE;
}

guint
ngf_vibrator_start (NgfVibrator *self, const char *name)
{
    (void) self;
    (void) name;

    return 0;
}

void
ngf_vibrator_stop (NgfVibrator *self, gint id)
{
    (void) self;
    (void) id;
}
