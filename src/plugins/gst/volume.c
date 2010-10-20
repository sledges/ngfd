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

#include <stdlib.h>
#include <string.h>

#include "volume.h"

static Volume*  volume_new           ();

static Volume*
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

static gboolean
_parse_profile_key (const char *key, gchar **out_profile, gchar **out_key)
{
    gchar **split = NULL;
    gboolean ret = FALSE;

    if (key == NULL)
        return FALSE;

    split = g_strsplit (key, "@", 2);
    if (split[0] == NULL) {
        ret = FALSE;
        goto done;
    }

    *out_key = g_strdup (split[0]);
    *out_profile = g_strdup (split[1]);
    ret = TRUE;

done:
    g_strfreev (split);
    return ret;
}

static gchar*
_strip_prefix (const gchar *str, const gchar *prefix)
{
    if (!g_str_has_prefix (str, prefix))
        return NULL;

    size_t prefix_length = strlen (prefix);
    return g_strdup (str + prefix_length);
}

Volume*
create_volume (const gchar *str)
{
    Volume *volume = NULL;
    gchar *stripped = NULL;
    gchar **split = NULL;
    gchar **item = NULL;
    gint i = 0;

    if (str == NULL)
        return NULL;

    if (g_str_has_prefix (str, "profile:")) {
        stripped = _strip_prefix (str, "profile:");

        volume       = volume_new ();
        volume->type = VOLUME_TYPE_PROFILE;

        if (!_parse_profile_key (stripped, &volume->profile, &volume->key)) {
            g_free (stripped);
            volume_free (volume);
            return NULL;
        }

        g_free (stripped);
    }
    else if (g_str_has_prefix (str, "fixed:")) {
        stripped = _strip_prefix (str, "fixed:");

        volume        = volume_new ();
        volume->type  = VOLUME_TYPE_FIXED;
        volume->level = atoi (stripped);

        g_free (stripped);
    }
    else if (g_str_has_prefix (str, "linear:")) {
        stripped = _strip_prefix (str, "linear:");

        volume             = volume_new ();
        volume->type       = VOLUME_TYPE_LINEAR;
        volume->level = 100;

        split = g_strsplit (stripped, ";", -1);
        if (split[0] == NULL) {
            g_strfreev (split);
            g_free (stripped);
            volume_free (volume);
            return NULL;
        }

        item = split;

        for (i=0;i<3;i++) {
            if (*item == NULL) {
                g_strfreev (split);
                g_free (stripped);
                volume_free (volume);
                return NULL;
            }
            volume->linear[i] = atoi (*item);        
            item++;
        }

        g_strfreev (split);
        g_free (stripped);
    }

    return volume;
}