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

#include <glib.h>
#include <getopt.h>

#include <ngf/log.h>
#include "core-internal.h"

typedef struct _AppData
{
    GMainLoop *loop;
    NCore     *core;
} AppData;

static gboolean
parse_cmdline (int argc, char **argv)
{
    int opt, opt_index;
    int level = N_LOG_LEVEL_NONE;

    static struct option long_opts[] = {
        { "verbose", 0, 0, 'v' },
        { 0, 0, 0, 0 }
    };

    while ((opt = getopt_long (argc, argv, "v", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 'v':
                if (level)
                    level--;
                break;

            default:
                break;
        }
    }

    n_log_set_level (level);

    return TRUE;
}

int
main (int argc, char *argv[])
{
    AppData *app = NULL;

    n_log_initialize (N_LOG_LEVEL_NONE);

    if (!parse_cmdline (argc, argv))
        return 1;

    app = g_new0 (AppData, 1);
    app->loop = g_main_loop_new (NULL, 0);
    app->core = n_core_new (&argc, argv);

    if (!n_core_initialize (app->core))
        return 1;

    g_main_loop_run   (app->loop);
    n_core_shutdown   (app->core);
    n_core_free       (app->core);
    g_main_loop_unref (app->loop);

    return 0;
}
