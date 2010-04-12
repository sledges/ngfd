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

#define NGF_VIBRA_POLL_TIMEOUT 500  // Poll frequency in ms for vibra completition

typedef struct _NgfVibrator NgfVibrator;

NgfVibrator*    ngf_vibrator_create   ();
void            ngf_vibrator_destroy  (NgfVibrator *self);
gboolean        ngf_vibrator_register (NgfVibrator *self, const char *name, const char *filename, gint pattern_id);
guint           ngf_vibrator_start    (NgfVibrator *self, const char *name);
void            ngf_vibrator_stop     (NgfVibrator *self, gint id);
gboolean        ngf_vibrator_is_completed     (NgfVibrator *self, gint id);
gboolean        ngf_vibrator_is_repeating    (NgfVibrator *self, const char *name);

#endif /* NGF_VIBRATOR_H */
