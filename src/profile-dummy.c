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

#include <stdlib.h>
#include "profile.h"

int
profile_create (Context *context)
{
    (void) context;
    return TRUE;
}

int
profile_resolve (Context *context)
{
    (void) context;
    return TRUE;
}

void
profile_destroy (Context *context)
{
    (void) context;
}


