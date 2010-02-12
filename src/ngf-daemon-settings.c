/*
 * ngfd - Non-graphical feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation. All rights reserved.
 *
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating,
 * any or all of this material requires the prior written consent of
 * Nokia Corporation. This material also contains confidential
 * information which may not be disclosed to others without the prior
 * written consent of Nokia.
 */

#include "ngf-daemon.h"
#include "ngf-conf.h"
#include "ngf-event-prototype.h"

#define STREAM_RESTORE_ID "module-stream-restore.id"

static gboolean
_profile_key_get (NgfConf *c, const char *group, const char *key, gchar **out_key, gchar **out_profile)
{
    gchar *value = NULL, **split = NULL, **p = NULL;

    ngf_conf_get_string (c, group, key, &value, NULL);
    if (value == NULL)
        return FALSE;

    split = g_strsplit (value, "@", 0);
    if (split) {
        p = split;
        *out_key = g_strdup (*p);
        ++p;

        if (*p == NULL)
            *out_profile = NULL;
        else
            *out_profile = g_strdup (*p);

        g_strfreev (split);
        return TRUE;
    }

    *out_key = NULL;
    *out_profile = NULL;

    return FALSE;
}

static gchar*
_strip_prefix (const gchar *str, const gchar *prefix)
{
    if (!g_str_has_prefix (str, prefix))
        return NULL;

    size_t prefix_length = strlen (prefix);
    return g_strdup (str + prefix_length);
}

static void
_parse_stream_properties (NgfConf *c, const char *group, const char *prefix, pa_proplist **proplist)
{
    gchar **keys = NULL, **k = NULL, *stream_prop = NULL, *stream_value = NULL;
    pa_proplist *p = NULL;
    gboolean has_props = FALSE;

    /* Get the stream properties */
    keys = ngf_conf_get_keys (c, group);

    p = pa_proplist_new ();
    for (k = keys; *k; k++) {
        if ((stream_prop = _strip_prefix (*k, prefix)) != NULL) {
            ngf_conf_get_string (c, group, *k, &stream_value, NULL);

            if (stream_value != NULL) {
               pa_proplist_sets (p, stream_prop, stream_value);
               has_props = TRUE;
            }

            g_free (stream_value);
            g_free (stream_prop);
        }
    }

    g_strfreev (keys);
    *proplist = p;
}

static void
_parse_volume_pattern (NgfConf *c, const char *group, const char *name, NgfVolumeController **controller)
{
    gint *value_list = NULL;
    gint num_values, i, step_time, step_value;

    NgfVolumeController *vc = NULL;
    gboolean has_steps = FALSE;

    if (name == NULL)
        return;

    ngf_conf_get_integer_list (c, group, name, &value_list, &num_values);
    if (value_list == NULL)
        return;

    vc = ngf_volume_controller_new ();

    for (i = 0; i < num_values; ) {
        if (i+1 >= num_values)
	       break;

        step_time = value_list[i];
        step_value = value_list[i+1];
        has_steps = TRUE;

        ngf_volume_controller_add_step (vc, step_time, step_value);
	    i += 2;
    }

    g_free (value_list);

    if (has_steps) {
        *controller = vc;
        return;
    }

    ngf_volume_controller_free (vc);
    *controller = NULL;
}

static void
_configuration_parse_proto (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    char **keys = NULL, **k = NULL;
    gchar *stream_prop = NULL, *stream_value = NULL, *default_role = NULL;

    if (name == NULL)
        return;

    default_role = g_strdup_printf ("x-maemo-ngf-%s", name);

    NgfEventPrototype *proto = NULL;
    proto = ngf_event_prototype_new ();

    ngf_conf_get_integer  (c, group, "max_length", &proto->max_length, 0);
    ngf_conf_get_boolean  (c, group, "tone_repeat", &proto->tone_repeat, FALSE);
    ngf_conf_get_integer  (c, group, "tone_repeat_count", &proto->tone_repeat_count, 0);
    ngf_conf_get_string   (c, group, "tone_filename", &proto->tone_filename, NULL);
    _profile_key_get      (c, group, "tone_key", &proto->tone_key, &proto->tone_profile);

    ngf_conf_get_integer  (c, group, "volume_set", &proto->volume_set, -1);
    ngf_conf_get_string   (c, group, "volume_role", &proto->volume_role, NULL);
    _profile_key_get      (c, group, "volume_key", &proto->volume_key, &proto->volume_profile);
    _parse_volume_pattern (c, group, "volume_pattern", &proto->volume_controller);

    _parse_stream_properties (c, group, "stream.", &proto->stream_properties);

    /* If no override volume role specified, let's use the default role. Also
       set the role to the stream properties. */

    if (proto->volume_role == NULL)
        proto->volume_role = default_role;
    else
        g_free (default_role);

    pa_proplist_sets (proto->stream_properties, STREAM_RESTORE_ID, proto->volume_role);

    /* Register the prototype */

    ngf_event_manager_register_prototype (self->event_manager, name, proto);
}

static void
_configuration_parse_event (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;

    if (name == NULL)
        return;

    NgfEventDefinition *def = NULL;
    def = ngf_event_definition_new ();

    ngf_conf_get_string (c, group, "long", &def->long_proto, NULL);
    ngf_conf_get_string (c, group, "short", &def->short_proto, NULL);

    ngf_event_manager_register_definition (self->event_manager, name, def);
}

gboolean
ngf_daemon_settings_load (NgfDaemon *self)
{
    static const char *conf_files[] = { "/etc/ngf/ngf.ini", "./ngf.ini", NULL };

    NgfConf *conf = NULL;
    const char **file = NULL;
    gboolean success = FALSE;

    conf = ngf_conf_new ();
    ngf_conf_add_group (conf, NGF_CONF_PARSE_PREFIX, "event", _configuration_parse_event, self);
    ngf_conf_add_group (conf, NGF_CONF_PARSE_PREFIX, "proto", _configuration_parse_proto, self);

    for (file = conf_files; *file; file++) {
        if (g_file_test (*file, G_FILE_TEST_EXISTS)) {
            if (ngf_conf_load (conf, *file))
                success = TRUE;
            break;
        }
    }

    ngf_conf_free (conf);

    return success;
}
