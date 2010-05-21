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

#include "vibrator.h"

struct _Vibrator
{
    int dummy;
};

Vibrator*
vibrator_create ()
{
    Vibrator *self = NULL;

    if ((self = g_new0 (Vibrator, 1)) == NULL)
        return NULL;

    return self;
}

void
vibrator_destroy (Vibrator *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

gboolean
vibrator_register (Vibrator *self, const char *name, const char *filename, gint pattern_id)
{
    (void) self;
    (void) name;
    (void) filename;
    (void) pattern_id;

    return TRUE;
}

guint
vibrator_start (Vibrator *self, const char *name)
{
    (void) self;
    (void) name;

    return 0;
}

void
vibrator_stop (Vibrator *self, gint id)
{
    (void) self;
    (void) id;
}

gboolean
vibrator_is_completed (Vibrator *self, gint id)
{
    (void) self;
    (void) id;

    return TRUE;
}

gboolean
vibrator_is_repeating (Vibrator *self, const char *name)
{
    (void) self;
    (void) name;

    return FALSE;
}
