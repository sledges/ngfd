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

#ifndef TONE_GENERATOR_H
#define TONE_GENERATOR_H

#include <glib.h>
#include <dbus/dbus.h>

gboolean tone_generator_start (DBusConnection *system_bus, guint pattern);
gboolean tone_generator_stop  (DBusConnection *system_bus, guint pattern);

#endif /* TONE_GENERATOR_H */
