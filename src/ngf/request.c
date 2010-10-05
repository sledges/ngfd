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


