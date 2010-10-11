/*
 * ngfd - Non-graphic feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation.
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef VIBRATOR_H
#define VIBRATOR_H

#include <glib.h>

typedef struct _Vibrator Vibrator;
typedef void (*VibratorCompletedCallback) (Vibrator *vibrator, gpointer userdata);

Vibrator* vibrator_create       ();
void      vibrator_destroy      (Vibrator *self);
gpointer  vibrator_load         (const char *filename);
guint     vibrator_start        (Vibrator *self, gpointer data, gint pattern_id, VibratorCompletedCallback callback, gpointer userdata);
void      vibrator_pause        (Vibrator *self, guint id);
void      vibrator_resume       (Vibrator *self, guint id);
void      vibrator_stop         (Vibrator *self, guint id);

#endif /* VIBRATOR_H */
