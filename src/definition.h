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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <glib.h>

typedef struct _Definition Definition;

struct _Definition
{
    gchar   *long_event;
    gchar   *short_event;
    gchar   *meeting_event;
};

Definition* definition_new  ();
void        definition_free (Definition *self);

#endif /* DEFINITION */
