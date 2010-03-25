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

#include <string.h>

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
_parse_controller_pattern (NgfConf *c, const char *group, const char *name, NgfController **controller)
{
    gint *value_list = NULL;
    gint num_values, i, step_time, step_value;

    NgfController *vc = NULL;
    gboolean has_steps = FALSE;

    if (name == NULL)
        return;

    ngf_conf_get_integer_list (c, group, name, &value_list, &num_values);
    if (value_list == NULL)
        return;

    vc = ngf_controller_new ();

    for (i = 0; i < num_values; ) {
        if (i+1 >= num_values)
	       break;

        step_time = value_list[i];
        step_value = value_list[i+1];
        has_steps = TRUE;

        ngf_controller_add_step (vc, step_time, step_value);
	    i += 2;
    }

    g_free (value_list);

    if (has_steps) {
        *controller = vc;
        return;
    }

    ngf_controller_free (vc);
    *controller = NULL;
}

static void
_configuration_parse_proto (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    NgfDaemon  *self         = (NgfDaemon*) userdata;
    gchar     **keys         = NULL;
    gchar     **k            = NULL;
    gchar      *stream_prop  = NULL;
    gchar      *stream_value = NULL;
    gchar      *default_role = NULL;

    NgfEventPrototype *p = NULL;

    if (name == NULL)
        return;

    default_role = g_strdup_printf ("x-maemo-ngf-%s", name);
    p = ngf_event_prototype_new ();

    /* Event */
    ngf_conf_get_integer      (c, group, "max_length", &p->max_length, 0);

    /* Audio */
    ngf_conf_get_boolean      (c, group, "audio_repeat", &p->audio_repeat, FALSE);
    ngf_conf_get_integer      (c, group, "audio_max_repeats", &p->audio_max_repeats, 0);
    ngf_conf_get_string       (c, group, "audio_tone_filename", &p->audio_tone_filename, NULL);
    _profile_key_get          (c, group, "audio_tone_profile", &p->audio_tone_key, &p->audio_tone_profile);
    ngf_conf_get_boolean      (c, group, "audio_silent", &p->audio_silent, FALSE);

    /* Fallback */
    ngf_conf_get_string       (c, group, "audio_fallback_filename", &p->audio_fallback_filename, NULL);
    _profile_key_get          (c, group, "audio_fallback_profile", &p->audio_fallback_key, &p->audio_fallback_profile);

    /* Audio volume */
    ngf_conf_get_integer      (c, group, "audio_volume_value", &p->audio_volume_value, -1);
    _profile_key_get          (c, group, "audio_volume_profile", &p->audio_volume_key, &p->audio_volume_profile);
    ngf_conf_get_string       (c, group, "audio_stream_role", &p->audio_stream_role, NULL);

    /* Tone generator */
    ngf_conf_get_boolean      (c, group, "audio_volume_repeat", &p->audio_volume_repeat, FALSE);
    _parse_controller_pattern (c, group, "audio_volume_pattern", &p->audio_volume_controller);

    ngf_conf_get_boolean      (c, group, "audio_tonegen_enabled", &p->audio_tonegen_enabled, FALSE);
    ngf_conf_get_integer      (c, group, "audio_tonegen_pattern", &p->audio_tonegen_pattern, -1);

    /* Stream properties */
    _parse_stream_properties  (c, group, "stream.", &p->stream_properties);

    /* Vibrator */
    ngf_conf_get_string       (c, group, "vibrator_pattern", &p->vibrator_pattern, NULL);

    /* LED */
    ngf_conf_get_string       (c, group, "led_pattern", &p->led_pattern, NULL);

    /* Backlight */
    ngf_conf_get_boolean      (c, group, "backlight_repeat", &p->backlight_repeat, FALSE);
    _parse_controller_pattern (c, group, "backlight_pattern", &p->backlight_controller);

    /* If no override volume role specified, let's use the default role. Also
       set the role to the stream properties. */

    if (p->audio_stream_role == NULL)
        p->audio_stream_role = default_role;
    else
        g_free (default_role);

    pa_proplist_sets (p->stream_properties, STREAM_RESTORE_ID, p->audio_stream_role);

    /* Register the prototype */
    ngf_daemon_register_prototype (self, name, p);
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

    ngf_daemon_register_definition (self, name, def);
}

static void
_configuration_parse_vibrator (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;

    gchar *filename = NULL;
    gint pattern_id = 0;

    if (name == NULL)
        return;

    ngf_conf_get_string  (c, group, "filename", &filename, NULL);
    ngf_conf_get_integer (c, group, "pattern_id", &pattern_id, -1);

    /* Register the vibrator pattern */
    if (self->context.vibrator)
        ngf_vibrator_register (self->context.vibrator, name, filename, pattern_id);

    g_free (filename);
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
    ngf_conf_add_group (conf, NGF_CONF_PARSE_PREFIX, "vibrator", _configuration_parse_vibrator, self);

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
