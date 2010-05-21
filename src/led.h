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

#ifndef LED_H
#define LED_H

#include <glib.h>
#include <dbus/dbus.h>

gboolean led_activate_pattern (DBusConnection *system_bus, const char *pattern);
gboolean led_deactive_pattern (DBusConnection *system_bus, const char *pattern);

#endif /* LED_H */
