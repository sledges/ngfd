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

#include "definition.h"

Definition*
definition_new ()
{
    return (Definition*) g_new0 (Definition, 1);
}

void
definition_free (Definition *self)
{
    if (self == NULL)
        return;

    g_free (self->long_event);
    g_free (self->short_event);
    g_free (self->meeting_event);
    g_free (self);
}
