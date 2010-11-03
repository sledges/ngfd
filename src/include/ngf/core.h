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

typedef struct _NCore NCore;

#include <glib.h>

#include <ngf/core-hooks.h>
#include <ngf/hook.h>
#include <ngf/sinkinterface.h>
#include <ngf/context.h>

NContext*        n_core_get_context  (NCore *core);
GList*           n_core_get_requests (NCore *core);
NSinkInterface** n_core_get_sinks    (NCore *core);
GList*           n_core_get_events   (NCore *core);

int              n_core_connect      (NCore *core, NCoreHook hook, int priority, NHookCallback callback, void *userdata);
void             n_core_disconnect   (NCore *core, NCoreHook hook, NHookCallback callback, void *userdata);

#endif /* N_CORE_H */
