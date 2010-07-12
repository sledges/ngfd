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

#include "vibration-pattern.h"

VibrationPattern*
vibration_pattern_new ()
{
    return (VibrationPattern*) g_try_malloc0 (sizeof (VibrationPattern));
}

void
vibration_pattern_free (VibrationPattern *pattern)
{
    g_free (pattern->key);
    g_free (pattern->profile);
    g_free (pattern->filename);
    g_free (pattern->data);
    g_free (pattern);
}

gboolean
vibration_pattern_equals (VibrationPattern *a, VibrationPattern *b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type) {
        case VIBRATION_PATTERN_TYPE_FILENAME:
            if (a->filename && b->filename && g_str_equal (a->filename, b->filename)) {
                if (a->pattern == b->pattern)
                    return TRUE;
            }
            break;

        case VIBRATION_PATTERN_TYPE_PROFILE:
            if (a->key == NULL || b->key == NULL)
                return FALSE;

            if (g_str_equal (a->key, b->key)) {
                if (a->profile && b->profile && g_str_equal (a->profile, b->profile))
                    return TRUE;

                if (a->profile == NULL && b->profile == NULL)
                    return TRUE;

                return FALSE;
            }
            break;

         case VIBRATION_PATTERN_TYPE_INTERNAL:
            if (a->pattern == b->pattern)
                return TRUE;
            break;

        default:
            break;
    }

    return FALSE;
}

void
vibration_pattern_array_free (VibrationPattern **array)
{
    VibrationPattern **iter = NULL;

    if (array == NULL)
        return;

    for (iter = array; *iter; ++iter) {
        vibration_pattern_free (*iter);
        *iter = NULL;
    }

    g_free (array);
}
