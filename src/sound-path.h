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

#ifndef SOUND_PATH_H
#define SOUND_PATH_H

#include <glib.h>

enum
{
    SOUND_PATH_TYPE_FILENAME,
    SOUND_PATH_TYPE_PROFILE
};

typedef struct _SoundPath SoundPath;

struct _SoundPath
{
    guint   type;       /* SOUND_PATH_TYPE_FILENAME or SOUND_PATH_TYPE_PROFILE */
    gchar  *filename;   /* absolute path to sound file */
    gchar  *key;
    gchar  *profile;
};

SoundPath* sound_path_new        ();
void       sound_path_free       (SoundPath *s);
gboolean   sound_path_equals     (SoundPath *a, SoundPath *b);
void       sound_path_array_free (SoundPath **array);

#endif /* SOUND_PATH_H */
