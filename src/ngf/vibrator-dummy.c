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

#include <stdio.h>

#include "log.h"
#include "vibrator.h"

#define VibeUInt8 unsigned char

struct _Vibrator
{
    int dummy; /* dummy */
};

Vibrator*
vibrator_create ()
{
    Vibrator *vibrator = NULL;

    if ((vibrator = g_new0 (Vibrator, 1)) == NULL)
        goto failed;

    return vibrator;

failed:
    vibrator_destroy (vibrator);
    return NULL;
}

void
vibrator_destroy (Vibrator *vibrator)
{
    if (vibrator == NULL)
        return;

    g_free (vibrator);
}

gpointer
vibrator_load (const char *filename)
{
    FILE *fp = NULL;
    long pattern_size = 0;
    size_t bytes_read = 0;
    VibeUInt8 *data = NULL;

    if (filename == NULL)
        goto failed;

    if ((fp = fopen (filename, "rb")) == NULL)
        goto failed;

    fseek (fp, 0L, SEEK_END);
    pattern_size = ftell (fp);
    fseek (fp, 0L, SEEK_SET);

    if (pattern_size > 0 && ((data = g_new (VibeUInt8, pattern_size)) != NULL)) {
        bytes_read = fread (data, sizeof (VibeUInt8), pattern_size, fp);
        if (bytes_read != pattern_size)
            goto failed;

        fclose (fp);

        return (gpointer)data;
    }

failed:
    if (data) {
        g_free (data);
        data = NULL;
    }

    if (fp) {
        fclose (fp);
        fp = NULL;
    }

    return NULL;
}

guint
vibrator_start (Vibrator *vibrator, gpointer data, gint pattern_id, VibratorCompletedCallback callback, gpointer userdata)
{
    (void) vibrator;
    (void) data;
    (void) pattern_id;
    (void) callback;
    (void) userdata;

    return 1;
}

void
vibrator_stop (Vibrator *vibrator, guint id)
{
    (void) vibrator;
    (void) id;
}
