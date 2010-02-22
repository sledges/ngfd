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

#ifndef NGF_TONE_MAPPER_H
#define NGF_TONE_MAPPER_H

#include <glib.h>

typedef struct _NgfToneMapper NgfToneMapper;

NgfToneMapper*  ngf_tone_mapper_create ();
void            ngf_tone_mapper_destroy (NgfToneMapper *self);

const char*     ngf_tone_mapper_get_tone (NgfToneMapper *self, const char *uri);

#endif /* NGF_TONE_MAPPER_H */
