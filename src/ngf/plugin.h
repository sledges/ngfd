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

#ifndef N_PLUGIN_H
#define N_PLUGIN_H

#include <ngf/log.h>
#include <ngf/proplist.h>

typedef struct _NPlugin NPlugin;

const NProplist* n_plugin_get_params (NPlugin *plugin);

#define N_PLUGIN_NAME(p_name)                   \
    const char* n_plugin__get_name () {         \
        return p_name;                          \
    }

#define N_PLUGIN_AUTHOR(p_author)               \
    const char* n_plugin__get_author () {       \
        return p_author;                        \
    }

#define N_PLUGIN_DESCRIPTION(p_desc)            \
    const char* n_plugin__get_desc () {         \
        return p_desc;                          \
    }

#define N_PLUGIN_VERSION(p_version)             \
    const char* n_plugin__get_version () {      \
        return p_version;                       \
    }

#define N_PLUGIN_LOAD(p_plugin)                 \
    int n_plugin__load (NPlugin* p_plugin)

#define N_PLUGIN_UNLOAD(p_plugin)               \
    void n_plugin__unload (NPlugin* p_plugin)

#endif /* N_PLUGIN_H */
