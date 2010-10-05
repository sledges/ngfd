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

#ifndef VIBRATION_PATTERN_H
#define VIBRATION_PATTERN_H

#include <glib.h>

enum
{
    VIBRATION_PATTERN_TYPE_FILENAME,
    VIBRATION_PATTERN_TYPE_PROFILE,
    VIBRATION_PATTERN_TYPE_INTERNAL
};

typedef struct _VibrationPattern VibrationPattern;

struct _VibrationPattern
{
    guint     type;         /* VIBRATION_PATTERN_TYPE_FILENAME or VIBRATION_PATTERN_TYPE_INTERNAL */
    gchar    *filename;     /* absolute path to IVT filename */
    guint     pattern;      /* pattern id within the IVT file */
    gchar    *key;          /* profile key */
    gchar    *profile;      /* profile to get the key from */
    gpointer  data;         /* vibration pattern data from the IVT file */
};

VibrationPattern* vibration_pattern_new        ();
void              vibration_pattern_free       (VibrationPattern *pattern);
gboolean          vibration_pattern_equals     (VibrationPattern *a, VibrationPattern *b);
void              vibration_pattern_array_free (VibrationPattern **array);

#endif /* VIBRATION_PATTERN_H */
