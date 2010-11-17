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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef N_HOOK_H
#define N_HOOK_H

#include <glib.h>

/**
 * Enum defining the order in which callbacks are executed.
 */
typedef enum _NHookPriority
{
    N_HOOK_PRIORITY_LAST    = -100,
    N_HOOK_PRIORITY_LOW     = -10,
    N_HOOK_PRIORITY_DEFAULT = 0,
    N_HOOK_PRIORITY_HIGH    = 10,
    N_HOOK_PRIORITY_FIRST   = 100
} NHookPriority;

/** Internal hook structure. */
typedef struct _NHook
{
    gchar *name;
    GList *slots;
} NHook;

/** Hook callback function */
typedef void (*NHookCallback) (NHook *hook, void *data, void *userdata);

/** Initializes hook structure
 * @param hook Hook.
 */
void n_hook_init       (NHook *hook);

/** Connect callback function to hook
 * @param hook Hook.
 * @param priority Priority of the callback function.
 * @param callback Callback function.
 * @param userdata Userdata.
 * @return TRUE if success.
 * @see NHookPriority
 * @see NHookCallback
 */
int  n_hook_connect    (NHook *hook, int priority, NHookCallback callback, void *userdata);

/** Disconnects callback function from hook
 * @param hook Hook.
 * @param callback Callback function.
 * @param userdata Userdata.
 */
void n_hook_disconnect (NHook *hook, NHookCallback callback, void *userdata);

/** Executes callback functions associated with hook
 * @param hook Hook.
 * @param data Data to pass the callback functions as userdata.
 * @return TRUE if success.
 */
int  n_hook_fire       (NHook *hook, void *data);

#endif /* N_HOOK_H */
