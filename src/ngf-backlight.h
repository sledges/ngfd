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

#ifndef NGF_BACKLIGHT_H
#define NGF_BACKLIGHT_H

#include <glib.h>

typedef struct _NgfBacklight NgfBacklight;

NgfBacklight*   ngf_backlight_create   ();
void            ngf_backlight_destroy  (NgfBacklight *self);
gboolean        ngf_backlight_start    (NgfBacklight *self, gboolean unlock);
void            ngf_backlight_stop     (NgfBacklight *self);

#endif /* NGF_BACKLIGHT_H */
