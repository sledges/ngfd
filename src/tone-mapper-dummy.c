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

#include "tone-mapper.h"

struct _ToneMapper
{
    int dummy;
};

ToneMapper*
tone_mapper_create ()
{
    ToneMapper *self = NULL;

    if ((self = g_new0 (ToneMapper, 1)) == NULL) {
        goto failed;
    }

    return self;

failed:
    tone_mapper_destroy (self);
    return NULL;
}

void
tone_mapper_destroy (ToneMapper *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

const char*
tone_mapper_get_tone (ToneMapper *self, const char *uri)
{
    (void) self;
    (void) uri;

    return NULL;
}
