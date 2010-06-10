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

#ifndef PROFILE_H
#define PROFILE_H

#include <glib.h>
#include "context.h"

int  profile_create    (Context *context);
int  profile_resolve   (Context *context);
int  profile_reconnect (Context *context);
void profile_destroy   (Context *context);

#endif /* PROFILE_H */
