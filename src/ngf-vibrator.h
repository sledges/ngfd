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

#ifndef NGF_VIBRATOR_H
#define NGF_VIBRATOR_H

#include <glib.h>

typedef struct _NgfVibrator NgfVibrator;

struct _NgfVibrator
{
    int tmp;
};

NgfVibrator*    ngf_vibrator_create ();
void            ngf_vibrator_destroy (NgfVibrator *self);

guint           ngf_vibrator_play_pattern (NgfVibrator *self, const char *pattern);
void            ngf_vibrator_stop_pattern (NgfVibrator *self, guint pattern_id);

#endif /* NGF_VIBRATOR_H */
