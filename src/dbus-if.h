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

#ifndef DBUS_IF_H
#define DBUS_IF_H

#include <glib.h>
#include "context.h"

int  dbus_if_create               (Context *context);
void dbus_if_destroy              (Context *context);
void dbus_if_send_status          (Context *context, guint id, guint status);
void dbus_if_send_resource_update (Context *context, guint id, gboolean audio, gboolean vibra, gboolean leds, gboolean backlight);

#endif /* DBUSIF_H */
