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

#ifndef LED_H
#define LED_H

#include <glib.h>

typedef struct _Led Led;

Led*  led_create  ();
void  led_destroy (Led *self);
guint led_start   (Led *self, const gchar *pattern);
void  led_stop    (Led *self, guint id);

#endif /* LED_H */
