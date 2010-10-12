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

#include "sinkinterface-internal.h"
#include "core-internal.h"

void
n_sink_interface_synchronize (NSinkInterface *iface, NRequest *request)
{
    if (!iface || !request)
        return;

    n_core_synchronize_sink (iface->core, iface, request);
}

void
n_sink_interface_complete (NSinkInterface *iface, NRequest *request)
{
    if (!iface || !request)
        return;

    n_core_complete_sink (iface->core, iface, request);
}

void
n_sink_interface_fail (NSinkInterface *iface, NRequest *request)
{
    if (!iface || !request)
        return;

    n_core_fail_sink (iface->core, iface, request);
}
