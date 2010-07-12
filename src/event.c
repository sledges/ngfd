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

#include "log.h"
#include "properties.h"
#include "event.h"

Event*
event_new ()
{
    Event *event = NULL;

    if ((event = g_new0 (Event, 1)) == NULL)
        return NULL;

    if ((event->properties = properties_new ()) == NULL) {
        g_free (event);
        return NULL;
    }

    return event;
}

void
event_free (Event *event)
{
    if (event == NULL)
        return;

    if (event->sounds) {
        g_list_free (event->sounds);
        event->sounds = NULL;
    }

    if (event->patterns) {
        g_list_free (event->patterns);
        event->patterns = NULL;
    }

    g_free (event->event_id);
    g_free (event->led_pattern);
    g_free (event);
}

void
event_dump (Event *event)
{
    properties_dump (event->properties);
}
