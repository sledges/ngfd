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

#include <glib.h>

typedef struct _ToneMapper ToneMapper;

ToneMapper* tone_mapper_create   ();
void        tone_mapper_destroy  (ToneMapper *self);
const char* tone_mapper_get_tone (ToneMapper *self, const char *uri);

#endif /* TONE_MAPPER_H */
