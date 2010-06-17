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

#ifndef VOLUME_CONTROLLER_H
#define VOLUME_CONTROLLER_H

#include "context.h"
#include "volume.h"

int  volume_controller_create     (Context *context);
void volume_controller_destroy    (Context *context);
int  volume_controller_update     (Context *context, Volume *volume);
int  volume_controller_update_all (Context *context);

#endif /* VOLUME_CONTROLLER_H */
