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

#include "ngf-controller.h"
#include "ngf-backlight.h"

struct _NgfBacklight
{
    int dummy;
};


NgfBacklight*
ngf_backlight_create ()
{
    NgfBacklight *self = NULL;

    if ((self = g_new0 (NgfBacklight, 1)) == NULL)
        goto failed;

    return self;

failed:
    ngf_backlight_destroy (self);
    return NULL;
}

void
ngf_backlight_destroy (NgfBacklight *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

void
ngf_backlight_register (NgfBacklight *self,
                        const char   *name,
                        const char   *pattern,
                        gboolean      repeat)
{
    return;
}

guint
ngf_backlight_start (NgfBacklight *self,
                     const char   *name)
{
    return 1;
}

void
ngf_backlight_stop (NgfBacklight *self,
                    guint         id)
{
    return;
}
