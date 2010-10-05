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

int
tone_mapper_create (Context *context)
{
    (void) context;

    return TRUE;
}

int
tone_mapper_reconnect (Context *context)
{
    (void) context;

    return TRUE;
}

void
tone_mapper_destroy (Context *context)
{
    (void) context;

    return;
}

const char*
tone_mapper_get_tone (Context *context, const char *orig)
{
    (void) context;
    (void) orig;

    return NULL;
}
