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

#ifndef TONEGEN_H
#define TONEGEN_H

#include <glib.h>

typedef struct _ToneGenerator ToneGenerator;

ToneGenerator* tone_generator_create ();
void           tone_generator_destroy (ToneGenerator *self);

guint          tone_generator_start (ToneGenerator *self, guint pattern);
void           tone_generator_stop (ToneGenerator *self, guint id);

#endif /* TONEGEN_H */
