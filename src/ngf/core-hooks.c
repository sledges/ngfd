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

#include <ngf/core-hooks.h>

const char*
n_core_hook_to_string (NCoreHook hook)
{
    switch (hook) {
        case N_CORE_HOOK_INIT_DONE:
            return "init_done";
        case N_CORE_HOOK_NEW_REQUEST:
            return "new_request";
        case N_CORE_HOOK_TRANSFORM_PROPERTIES:
            return "transform_properties";
        case N_CORE_HOOK_FILTER_SINKS:
            return "filter_sinks";
        default:
            break;
    }

    return "unknown";
}
