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

#include "controller.h"
#include "backlight.h"

struct _Backlight
{
    int dummy;
};


Backlight*
backlight_create ()
{
    Backlight *self = NULL;

    if ((self = g_new0 (Backlight, 1)) == NULL)
        goto failed;

    return self;

failed:
    backlight_destroy (self);
    return NULL;
}

void
backlight_destroy (Backlight *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

gboolean
backlight_start (Backlight *self,
                     gboolean      unlock)
{
    (void) self;
    (void) unlock;

    return TRUE;
}

void
backlight_stop (Backlight *self)
{
    (void) self;
}
