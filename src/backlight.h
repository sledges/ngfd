/*
 * ngfd - Non-graphic feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation.
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <glib.h>
#include <dbus/dbus.h>

gboolean backlight_unlock_tklock        (DBusConnection *system_bus);
gboolean backlight_display_on           (DBusConnection *system_bus);
gboolean backlight_prevent_blank        (DBusConnection *system_bus);
gboolean backlight_cancel_prevent_blank (DBusConnection *system_bus);

#endif /* BACKLIGHT_H */
