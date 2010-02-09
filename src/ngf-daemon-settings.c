#include "ngf-daemon.h"
#include "ngf-conf.h"
#include "ngf-event-prototype.h"

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

    if (has_props) {
        *proplist = p;
        return;
    }

    pa_proplist_free (p);
    *proplist = NULL;
}

static void
_configuration_parse_event (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    char **keys = NULL, **k = NULL;
    gchar *stream_prop = NULL, *stream_value = NULL;

    if (name == NULL)
        return;

    NgfEventPrototype *proto = NULL;
    proto = ngf_event_prototype_new ();

    ngf_conf_get_integer (c, group, "max_length", &proto->max_length, 0);
    ngf_conf_get_string  (c, group, "tone_filename", &proto->tone_filename, NULL);
    _profile_key_get     (c, group, "tone_key", &proto->tone_key, &proto->tone_profile);
    _profile_key_get     (c, group, "volume_key", &proto->volume_key, &proto->volume_profile);

    _parse_stream_properties (c, group, "stream.", &proto->stream_properties);

    ngf_event_manager_register_prototype (self->event_manager, name, proto);
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

    for (file = conf_files; *file; file++) {
        if (g_file_test (*file, G_FILE_TEST_EXISTS)) {
            g_print ("trying to load = %s\n", *file);
            if (ngf_conf_load (conf, *file))
                success = TRUE;
            break;
        }
    }

    ngf_conf_free (conf);

    return success;
}
