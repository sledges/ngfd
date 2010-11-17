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

#ifndef VOLUME_H
#define VOLUME_H

#include <glib.h>

enum
{
    VOLUME_TYPE_FIXED,
    VOLUME_TYPE_PROFILE,
    VOLUME_TYPE_LINEAR
};

typedef struct _Volume Volume;

struct _Volume
{
    guint       type;           /* FIXED, PROFILE or LINEAR */
    gint        level;          /* volume level, range 0-100 */
    gint        linear[3];
    gchar      *role;           /* volume role */
    gchar      *key;
    gchar      *profile;
};

void     volume_free          (Volume *v);
void     volume_array_free    (Volume **array);
Volume* create_volume (const gchar *str);

#endif /* VOLUME_H */
