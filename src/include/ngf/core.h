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

#ifndef N_CORE_H
#define N_CORE_H

/** Internal core structure. */
typedef struct _NCore NCore;

#include <glib.h>

#include <ngf/core-hooks.h>
#include <ngf/hook.h>
#include <ngf/sinkinterface.h>
#include <ngf/context.h>

/**
 * Get context structure associated with core
 *
 * @param core Core.
 * @return NContext structure.
 */
NContext*        n_core_get_context  (NCore *core);

/**
 * Get list of active requests
 *
 * @param core Core.
 * @return Requests in GList type.
 */
GList*           n_core_get_requests (NCore *core);

/**
 * Get list of registered sinks
 *
 * @param core Core.
 * @return Array of NSinkInterfaces
 */
NSinkInterface** n_core_get_sinks    (NCore *core);

/**
 * Get list of known events
 *
 * @param core Core.
 * @return GList of events.
 */
GList*           n_core_get_events   (NCore *core);

/**
 * Connect callback function to hook
 *
 * @param core Core.
 * @param hook Hook to connect callback to.
 * @param priority Priority of callback function.
 * @param callback Callback function.
 * @param userdata Userdata.
 * @return TRUE if successful.
 * @see _NCoreHook
 * @see _NHookPriority
 * @see NHookCallback
 */
int              n_core_connect      (NCore *core, NCoreHook hook, int priority, NHookCallback callback, void *userdata);

/**
 * Disconnect callback function from hook
 *
 * @param core Core.
 * @param hook Hook to connect callback to.
 * @param callback Callback function.
 * @param userdata Userdata.
 * @see _NCoreHook
 * @see NHookCallback
 */
void             n_core_disconnect   (NCore *core, NCoreHook hook, NHookCallback callback, void *userdata);

#endif /* N_CORE_H */
