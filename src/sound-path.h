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
