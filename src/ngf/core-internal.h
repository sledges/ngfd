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
#include "sinkinterface-internal.h"
#include "inputinterface-internal.h"
#include "event-internal.h"
#include "request-internal.h"
#include "context-internal.h"

struct _NCore
{
    gchar            *conf_path;            /* configuration path */
    gchar            *plugin_path;          /* plugin path */

    GList            *required_plugins;     /* plugins to load (required) */
    GList            *plugins;              /* NPlugin* */

    NSinkInterface  **sinks;                /* sink interfaces registered */
    unsigned int      num_sinks;
    NInputInterface **inputs;               /* input interfaces registered */
    unsigned int      num_inputs;

    NContext         *context;              /* global context for broadcasting and sharing values */
    GHashTable       *events;               /* hash table of GList* containing NEvent* */
    GList            *requests;             /* active requests */

    gboolean          shutdown_done;        /* shutdown has been run. */
};

NCore*    n_core_new              (int *argc, char **argv);
void      n_core_free             (NCore *core);
int       n_core_initialize       (NCore *core);
void      n_core_shutdown         (NCore *core);

void      n_core_register_sink    (NCore *core, const NSinkInterfaceDecl *iface);
void      n_core_register_input   (NCore *core, const NInputInterfaceDecl *iface);
void      n_core_add_event        (NCore *core, NEvent *event);
NEvent*   n_core_evaluate_request (NCore *core, NRequest *request);

#endif /* N_CORE_INTERNAL_H */

