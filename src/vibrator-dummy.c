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
vibrator_stop (Vibrator *vibrator, gint id)
{
    (void) vibrator;
    (void) id;
}
