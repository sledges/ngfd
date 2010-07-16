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

#include "volume.h"

Volume*
volume_new ()
{
    Volume *v = NULL;

    v = (Volume*) g_try_malloc0 (sizeof (Volume));
    return v;
}

void
volume_free (Volume *v)
{
    g_free (v->role);
    g_free (v->key);
    g_free (v->profile);
    g_free (v);
}

gboolean
volume_equals (Volume *a, Volume *b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type) {
        case VOLUME_TYPE_FIXED:
            if (a->level == b->level)
                return TRUE;

            break;

        case VOLUME_TYPE_PROFILE:
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

        case VOLUME_TYPE_LINEAR:
            if ((a->linear[0] && b->linear[0]) &&
                (a->linear[1] && b->linear[1]) &&
                (a->linear[2] && b->linear[2]))
                return TRUE;

            break;

        default:
            break;
    }

    return FALSE;
}

void
volume_array_free (Volume **array)
{
    Volume **iter = NULL;

    if (array == NULL)
        return;

    for (iter = array; *iter; ++iter) {
        volume_free (*iter);
        *iter = NULL;
    }

    g_free (array);
}

static gchar*
cleanup_value (const gchar *str)
{
    gchar *cleaned = NULL, *p = NULL;

    if (!str)
        return g_strdup_printf ("any");

    cleaned = g_strdup (str);
    for (p = cleaned; *p != '\0'; ++p) {
        if (*p == '.') *p = '_';
    }

    return cleaned;
}

gboolean
volume_generate_role (Volume *volume)
{
    gchar *format_key = NULL, *format_profile = NULL;

    if (volume->role)
        return FALSE;

    switch (volume->type) {
        case VOLUME_TYPE_FIXED:
            volume->role = g_strdup_printf ("x-meego-fixed-%d", volume->level);
            break;

        case VOLUME_TYPE_PROFILE:
            format_profile = cleanup_value (volume->profile);
            format_key     = cleanup_value (volume->key);

            volume->role = g_strdup_printf ("x-meego-%s-%s", format_profile, format_key);

            g_free (format_key);
            g_free (format_profile);
            break;

        case VOLUME_TYPE_LINEAR:
            volume->role = g_strdup_printf ("x-meego-linear");
            break;

        default:
            return FALSE;
    }

    return TRUE;
}
