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

#ifndef N_CORE_INTERNAL_H
#define N_CORE_INTERNAL_H


#include <glib.h>

#include "core.h"
#include "plugin-internal.h"

struct _NCore
{
    gchar            *conf_path;            /* configuration path */
    gchar            *plugin_path;          /* plugin path */

    GList            *required_plugins;     /* plugins to load (required) */
    GList            *plugins;              /* NPlugin* */

    gboolean          shutdown_done;        /* shutdown has been run. */
};

NCore*    n_core_new            (int *argc, char **argv);
void      n_core_free           (NCore *core);

int       n_core_initialize     (NCore *core);
void      n_core_shutdown       (NCore *core);

#endif /* N_CORE_INTERNAL_H */

