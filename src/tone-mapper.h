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

#ifndef TONE_MAPPER_H
#define TONE_MAPPER_H

#include "context.h"

int         tone_mapper_create    (Context *context);
int         tone_mapper_reconnect (Context *context);
void        tone_mapper_destroy   (Context *context);
const char* tone_mapper_get_tone  (Context *context,  const char *orig);

#endif /* TONE_MAPPER_H */
