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

#include <glib.h>
#include "ngf-conf.h"

typedef struct _ParseRule
{
    NgfConfParseType     parse_type;
    gchar                *match;
    NgfConfParseCallback callback;
    gpointer             userdata;
} ParseRule;

static gchar*
_get_group_name (const gchar *group)
{
    gchar *ptr = NULL;

    ptr = (gchar*) group;
    while (*ptr != '\0' && *ptr != ' ')
        ptr++;

    if (*ptr == ' ')
        ptr++;

    if (*ptr == '\0')
        return NULL;

    return g_strdup (ptr);
}

static void
_match_rule (NgfConf *conf, const gchar *group)
{
    ParseRule *rule = NULL;
    GSList *iter = NULL;
    gchar *name = NULL;

    iter = conf->parse_rules;
    while (iter) {
        rule = (ParseRule*) iter->data;

        switch (rule->parse_type) {
            case NGF_CONF_PARSE_EXACT: {
                if (rule->callback && g_str_equal (group, rule->match)) {
                    rule->callback (conf, group, NULL, rule->userdata);
                }
                break;
            }

            case NGF_CONF_PARSE_PREFIX: {
                if (rule->callback && g_str_has_prefix (group, rule->match)) {
                    name = _get_group_name (group);
                    rule->callback (conf, group, name, rule->userdata);
                    g_free (name);
                }
                break;
            }

            default:
                break;
        }

        iter = g_slist_next (iter);
    }
}

NgfConf*
ngf_conf_new ()
{
    NgfConf *c = NULL;
    c = g_new0 (NgfConf, 1);
    if (!c)
        return NULL;

    return c;
}

static void
_parse_rule_free (gpointer data,
                  gpointer user_data)
{
    ParseRule *rule = (ParseRule*) data;
    g_free (rule->match);
    g_free (rule);
}

void
ngf_conf_free (NgfConf *conf)
{
    if (!conf)
        return;

    if (conf->parse_rules) {
        g_slist_foreach (conf->parse_rules, _parse_rule_free, NULL);
        g_slist_free (conf->parse_rules);
        conf->parse_rules = NULL;
    }

    g_free (conf);
}

gboolean
ngf_conf_load (NgfConf *conf, const gchar *filename)
{
    GError *error = NULL;
    gchar **groups = NULL, **group = NULL;

    if (!conf)
        return FALSE;

    conf->f = g_key_file_new ();
    if (!g_key_file_load_from_file (conf->f, filename, G_KEY_FILE_NONE, &error)) {
        g_warning ("%s: Unable to load configuration file: %s", __FUNCTION__, error->message);
        g_error_free (error);
        goto failed;
    }

    groups = g_key_file_get_groups (conf->f, NULL);
    if (groups) {
        group = groups;
        while (*group) {
            _match_rule (conf, *group);
            group++;
        }

        g_strfreev (groups);
    }

    g_key_file_free (conf->f);
    conf->f = NULL;
    return TRUE;

failed:
    g_key_file_free (conf->f);
    return FALSE;
}

void
ngf_conf_add_group (NgfConf *conf,
                    NgfConfParseType parse_type,
                    const gchar *match,
                    NgfConfParseCallback callback,
                    gpointer userdata)
{
    ParseRule *rule = NULL;

    if (!conf)
        return;

    rule = g_new (ParseRule, 1);
    rule->parse_type = parse_type;
    rule->match = g_strdup (match);
    rule->callback = callback;
    rule->userdata = userdata;

    conf->parse_rules = g_slist_append (conf->parse_rules, rule);
}

char**
ngf_conf_get_keys (NgfConf *conf, const gchar *group)
{
    GError *error = NULL;
    char **keys = NULL;

    if (!conf)
        return;

    keys = g_key_file_get_keys (conf->f, group, NULL, &error);
    if (keys == NULL) {
        g_error_free (error);
        return NULL;
    }

    return keys;
}

void
ngf_conf_get_boolean (NgfConf *conf, const gchar *group, const gchar *key, gboolean *value, gboolean def_value)
{
    GError *error = NULL;
    gboolean v;

    if (!conf)
        return;

    v = g_key_file_get_boolean (conf->f, group, key, &error);
    if (error) {
        g_error_free (error);
        v = def_value;
    }

    *value = v;
}

void
ngf_conf_get_string (NgfConf *conf, const gchar *group, const gchar *key, gchar **value, const gchar *def_value)
{
    GError *error = NULL;
    gchar *v = NULL;

    if (!conf)
        return;

    v = g_key_file_get_string (conf->f, group, key, &error);
    if (v == NULL) {
        g_error_free (error);
        v = g_strdup (def_value);
    }

    *value = v;
}

void
ngf_conf_get_integer (NgfConf *conf, const gchar *group, const gchar *key, gint *value, gint def_value)
{
    GError *error = NULL;
    gint v;

    if (!conf)
        return;

    v = g_key_file_get_integer (conf->f, group, key, &error);
    if (error) {
        g_error_free (error);
        v = -1;
    }

    *value = v;
}

void
ngf_conf_get_integer_list (NgfConf *conf, const gchar *group, const gchar *key, gint **value, gint *num_values)
{
    GError *error = NULL;
    gint *v = NULL, num_v;

    if (!conf)
        return;

    v = g_key_file_get_integer_list (conf->f, group, key, &num_v, &error);
    if (error)
        g_error_free (error);

    *value = v;
    *num_values = num_v;
}

