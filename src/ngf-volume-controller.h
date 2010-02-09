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

#ifndef NGF_VOLUME_CONTROLLER_H
#define NGF_VOLUME_CONTROLLER_H

#include <glib.h>

typedef struct _NgfVolumeController NgfVolumeController;

typedef gboolean (*NgfVolumeControllerCallback) (guint id, guint step_time, guint step_value, gpointer userdata);

NgfVolumeController* ngf_volume_controller_new ();
void    ngf_volume_controller_free (NgfVolumeController *self);
void    ngf_volume_controller_add_step (NgfVolumeController *self, guint step_time, guint step_value);
guint   ngf_volume_controller_start (NgfVolumeController *self, NgfVolumeControllerCallback callback, gpointer userdata);
void    ngf_volume_controller_stop (NgfVolumeController *self, guint id);

#endif /* NGF_VOLUME_CONTROLLER_H */
