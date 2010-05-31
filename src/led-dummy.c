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

#include "log.h"
#include "led.h"

gboolean
led_activate_pattern (DBusConnection *system_bus, const char *pattern)
{
    (void) system_bus;
    (void) pattern;

    return TRUE;
}

gboolean
led_deactivate_pattern (DBusConnection *system_bus, const char *pattern)
{
    (void) system_bus;
    (void) pattern;

    return TRUE;
}
