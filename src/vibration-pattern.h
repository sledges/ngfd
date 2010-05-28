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

#ifndef VIBRATION_PATTERN_H
#define VIBRATION_PATTERN_H

#include <glib.h>

enum
{
    VIBRATION_PATTERN_TYPE_FILENAME,
    VIBRATION_PATTERN_TYPE_INTERNAL
};

typedef struct _VibrationPattern VibrationPattern;

struct _VibrationPattern
{
    guint     type;         /* VIBRATION_PATTERN_TYPE_FILENAME or VIBRATION_PATTERN_TYPE_INTERNAL */
    gchar    *filename;     /* absolute path to IVT filename */
    guint     pattern;      /* pattern id within the IVT file */
    gpointer  data;         /* vibration pattern data from the IVT file */
};

VibrationPattern* vibration_pattern_new        ();
void              vibration_pattern_free       (VibrationPattern *pattern);
gboolean          vibration_pattern_equals     (VibrationPattern *a, VibrationPattern *b);
void              vibration_pattern_array_free (VibrationPattern **array);

#endif /* VIBRATION_PATTERN_H */
