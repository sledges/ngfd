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

#include "led.h"

struct _Led
{
    int dummy;
};

Led*
led_create ()
{
    Led *self = NULL;

    if ((self = g_new0 (Led, 1)) == NULL)
        return NULL;

    return self;
}

void
led_destroy (Led *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

guint
led_start (Led *self, const char *pattern)
{
    return 1;
}

void
led_stop (Led *self, guint id)
{
    return;
}