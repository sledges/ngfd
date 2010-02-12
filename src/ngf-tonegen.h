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

#ifndef NGF_TONEGEN_H
#define NGF_TONEGEN_H

#include <glib.h>

typedef struct _NgfTonegen NgfTonegen;

NgfTonegen*     ngf_tonegen_create ();
void            ngf_tonegen_destroy (NgfTonegen *self);

guint           ngf_tonegen_start (NgfTonegen *self, guint pattern);
void            ngf_tonegen_stop (NgfTonegen *self, guint id);

#endif /* NGF_TONEGEN_H */
