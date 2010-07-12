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

#include "sound-path.h"

SoundPath*
sound_path_new ()
{
    SoundPath *s = NULL;

    s = (SoundPath*) g_try_malloc0 (sizeof (SoundPath));
    return s;
}

void
sound_path_free (SoundPath *s)
{
    g_free (s->filename);
    g_free (s->key);
    g_free (s->profile);
    g_free (s);
}

gboolean
sound_path_equals (SoundPath *a, SoundPath *b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type) {
        case SOUND_PATH_TYPE_FILENAME:
            if (a->filename && b->filename && g_str_equal (a->filename, b->filename))
                return TRUE;
            break;

        case SOUND_PATH_TYPE_PROFILE:
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

        default:
            break;
    }

    return FALSE;
}

void
sound_path_array_free (SoundPath **array)
{
    SoundPath **iter = NULL;

    if (array == NULL)
        return;

    for (iter = array; *iter; ++iter) {
        sound_path_free (*iter);
        *iter = NULL;
    }

    g_free (array);
}
