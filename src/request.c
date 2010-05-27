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

#include <string.h>

#include "log.h"
#include "request.h"

Request*
request_new (Context *context, Event *event)
{
    Request *request = NULL;

    if (context == NULL || event == NULL)
        return NULL;

    if ((request = g_try_malloc0 (sizeof (Request))) == NULL)
        return NULL;

    request->context = context;
    request->event   = event;

    return request;
}

void
request_free (Request *request)
{
    if (request == NULL)
        return;

    if (request->custom_sound) {
        sound_path_free (request->custom_sound);
        request->custom_sound = NULL;
    }

    if (request->custom_pattern) {
        vibration_pattern_free (request->custom_pattern);
        request->custom_pattern = NULL;
    }

    g_free (request->name);
    g_free (request);
}

void
request_set_custom_sound (Request *request, const char *path)
{
    if (path == NULL || (path && !g_file_test (path, G_FILE_TEST_EXISTS)))
        return;

    request->custom_sound = sound_path_new ();
    request->custom_sound->type     = SOUND_PATH_TYPE_FILENAME;
    request->custom_sound->filename = g_strdup (path);
}


