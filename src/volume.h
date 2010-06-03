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

#ifndef VOLUME_H
#define VOLUME_H

#include <glib.h>
#include "controller.h"

enum
{
    VOLUME_TYPE_FIXED,
    VOLUME_TYPE_PROFILE,
    VOLUME_TYPE_CONTROLLER
};

typedef struct _Volume Volume;

struct _Volume
{
    guint       type;           /* FIXED, PROFILE or CONTROLLER */
    gint        level;          /* volume level, range 0-100 */
    gchar      *role;           /* volume role */
    gchar      *key;
    gchar      *profile;
    Controller *controller;     /* controller instance for the specific pattern */
};

Volume*  volume_new           ();
void     volume_free          (Volume *v);
gboolean volume_equals        (Volume *a, Volume *b);
void     volume_array_free    (Volume **array);
gboolean volume_generate_role (Volume *volume);

#endif /* VOLUME_H */
