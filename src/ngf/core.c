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

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "log.h"
#include "core-internal.h"

#define PATH_LEN 4096
#define LOG_CAT  "core: "

#define DEFAULT_CONF_PATH     "/etc/ngf"
#define DEFAULT_PLUGIN_PATH   "/usr/lib/ngf"
#define DEFAULT_CONF_FILENAME "ngf.ini"
#define PLUGIN_CONF_PATH      "plugins.d"
#define EVENT_CONF_PATH       "events.d"

static gchar*     n_core_get_path                 (const char *key, const char *default_path);
static NProplist* n_core_load_params              (NCore *core, const char *plugin_name);
static NPlugin*   n_core_load_plugin              (NCore *core, const char *plugin_name);
static void       n_core_unload_plugin            (NCore *core, NPlugin *plugin);



static gchar*
n_core_get_path (const char *key, const char *default_path)
{
    g_assert (default_path != NULL);

    const char *env_path = NULL;
    const char *source   = NULL;
    char        path[PATH_LEN];

    source = default_path;
    if (key && (env_path = getenv (key)) != NULL)
        source = env_path;

    strncpy (path, source, PATH_LEN);
    path[PATH_LEN - 1] = '\0';
    return g_strdup (path);
}

static NProplist*
n_core_load_params (NCore *core, const char *plugin_name)
{
    g_assert (core != NULL);
    g_assert (plugin_name != NULL);

    NProplist  *proplist  = NULL;
    GKeyFile   *keyfile   = NULL;
    gchar      *filename  = NULL;
    gchar      *full_path = NULL;
    gchar     **keys      = NULL;
    gchar     **iter      = NULL;
    GError     *error     = NULL;
    gchar      *value     = NULL;

    filename  = g_strdup_printf ("%s.ini", plugin_name);
    full_path = g_build_filename (core->conf_path, PLUGIN_CONF_PATH, filename, NULL);
    keyfile   = g_key_file_new ();

    if (!g_key_file_load_from_file (keyfile, full_path, G_KEY_FILE_NONE, &error)) {
        if (error->code & G_KEY_FILE_ERROR_NOT_FOUND) {
            N_WARNING (LOG_CAT "problem with configuration file '%s': %s",
                filename, error->message);
        }

        goto done;
    }

    keys = g_key_file_get_keys (keyfile, plugin_name, NULL, NULL);
    if (!keys) {
        N_WARNING (LOG_CAT "no group '%s' within configuration file '%s'",
            plugin_name, filename);
        goto done;
    }

    proplist = n_proplist_new ();
    for (iter = keys; *iter; ++iter) {
        if ((value = g_key_file_get_string (keyfile, plugin_name, *iter, NULL))) {
            N_DEBUG (LOG_CAT "parameter for '%s': %s = %s", plugin_name, *iter,
                value);
            n_proplist_set_string (proplist, *iter, value);
            g_free (value);
        }
    }

    g_strfreev (keys);

done:
    if (error)
        g_error_free (error);

    if (keyfile)
        g_key_file_free (keyfile);

    g_free          (full_path);
    g_free          (filename);

    return proplist;
}

static NPlugin*
n_core_load_plugin (NCore *core, const char *plugin_name)
{
    g_assert (core != NULL);
    g_assert (plugin_name != NULL);

    NPlugin *plugin    = NULL;
    gchar   *filename  = NULL;
    gchar   *full_path = NULL;

    filename  = g_strdup_printf ("libngfd_%s.so", plugin_name);
    full_path = g_build_filename (core->plugin_path, filename, NULL);

    if (!(plugin = n_plugin_load (full_path)))
        goto done;

    plugin->core   = core;
    plugin->params = n_core_load_params (core, plugin_name);

    if (!plugin->load (plugin))
        goto done;

    N_DEBUG (LOG_CAT "loaded plugin '%s'", plugin_name);

    g_free (full_path);
    g_free (filename);

    return plugin;

done:
    N_ERROR (LOG_CAT "unable to load plugin '%s'", plugin_name);

    if (plugin)
        n_plugin_unload (plugin);

    g_free (full_path);
    g_free (filename);

    return NULL;
}

static void
n_core_unload_plugin (NCore *core, NPlugin *plugin)
{
    g_assert (core != NULL);
    g_assert (plugin != NULL);

    N_DEBUG (LOG_CAT "unloading plugin '%s'", plugin->get_name ());
    plugin->unload (plugin);
    n_plugin_unload (plugin);
}

NCore*
n_core_new (int *argc, char **argv)
{
    NCore *core = NULL;

    (void) argc;
    (void) argv;

    core = g_new0 (NCore, 1);

    /* query the default paths */
    core->conf_path   = n_core_get_path ("NGF_CONF_PATH", DEFAULT_CONF_PATH);
    core->plugin_path = n_core_get_path ("NGF_PLUGIN_PATH", DEFAULT_PLUGIN_PATH);

    return core;
}

void
n_core_free (NCore *core)
{
    g_assert (core != NULL);

    if (!core->shutdown_done)
        n_core_shutdown (core);

    g_free (core->plugin_path);
    g_free (core->conf_path);
    g_free (core);
}

int
n_core_initialize (NCore *core)
{
    g_assert (core != NULL);
    g_assert (core->conf_path != NULL);
    g_assert (core->plugin_path != NULL);

    NSinkInterface **sink   = NULL;
    NPlugin         *plugin = NULL;
    GList           *p      = NULL;

    if (!core->required_plugins) {
        N_ERROR (LOG_CAT "no plugins to load defined in configuration");
        goto failed_init;
    }

    /* load all plugins */

    for (p = g_list_first (core->required_plugins); p; p = g_list_next (p)) {
        if (!(plugin = n_core_load_plugin (core, (const char*) p->data)))
            goto failed_init;

        core->plugins = g_list_append (core->plugins, plugin);
    }

    /* initialize all sinks. if no sinks, we're done. */

    if (!core->sinks) {
        N_ERROR (LOG_CAT "no plugin has registered sink interface");
        goto failed_init;
    }

    for (sink = core->sinks; *sink; ++sink) {
        if ((*sink)->funcs.initialize && !(*sink)->funcs.initialize (*sink)) {
            N_ERROR (LOG_CAT "sink '%s' failed to initialize", (*sink)->name);
            goto failed_init;
        }
    }

    return TRUE;

failed_init:
    return FALSE;
}

static void
unload_plugin_cb (gpointer data, gpointer userdata)
{
    NCore   *core   = (NCore*) userdata;
    NPlugin *plugin = (NPlugin*) data;
    n_core_unload_plugin (core, plugin);
}

void
n_core_shutdown (NCore *core)
{
    g_assert (core != NULL);

    NSinkInterface **sink = NULL;

    /* shutdown all sinks */

    if (core->sinks) {
        for (sink = core->sinks; *sink; ++sink) {
            if ((*sink)->funcs.shutdown)
                (*sink)->funcs.shutdown (*sink);
        }
    }

    if (core->plugins) {
        g_list_foreach (core->plugins, unload_plugin_cb, core);
        g_list_free (core->plugins);
        core->plugins = NULL;
    }

    if (core->required_plugins) {
        g_list_foreach (core->required_plugins, (GFunc) g_free, NULL);
        g_list_free (core->required_plugins);
        core->required_plugins = NULL;
    }

    core->shutdown_done = TRUE;
}

void
n_core_register_sink (NCore *core, const NSinkInterfaceDecl *iface)
{
    g_assert (core != NULL);
    g_assert (iface->name != NULL);
    g_assert (iface->play != NULL);
    g_assert (iface->stop != NULL);

    NSinkInterface *sink = NULL;
    sink = g_new0 (NSinkInterface, 1);
    sink->name  = iface->name;
    sink->core  = core;
    sink->funcs = *iface;

    core->num_sinks++;
    core->sinks = (NSinkInterface**) g_realloc (core->sinks,
        sizeof (NSinkInterface*) * (core->num_sinks + 1));

    core->sinks[core->num_sinks-1] = sink;
    core->sinks[core->num_sinks]   = NULL;

    N_DEBUG (LOG_CAT "sink interface '%s' registered", sink->name);
}

