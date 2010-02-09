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

#ifndef NGF_CONF_H
#define NGF_CONF_H

#include <glib.h>

typedef enum _NgfConfParseType
{
    NGF_CONF_PARSE_EXACT = 0,
    NGF_CONF_PARSE_PREFIX
} NgfConfParseType;

typedef struct _NgfConf
{
    GKeyFile *f;
    GSList   *parse_rules;
} NgfConf;

typedef void (*NgfConfParseCallback) (NgfConf *c, const gchar *group, const gchar *name, gpointer userdata);

NgfConf*    ngf_conf_new ();
void        ngf_conf_free (NgfConf *conf);

gboolean    ngf_conf_load (NgfConf *conf, const gchar *filename);

void        ngf_conf_add_group (NgfConf *conf, NgfConfParseType parse_type, const gchar *match, NgfConfParseCallback callback, gpointer userdata);

char**      ngf_conf_get_keys (NgfConf *conf, const gchar *group);
void        ngf_conf_get_boolean (NgfConf *conf, const gchar *group, const gchar *key, gboolean *value, gboolean def_value);
void        ngf_conf_get_string (NgfConf *conf, const gchar *group, const gchar *key, gchar **value, const gchar *def_value);
void        ngf_conf_get_integer (NgfConf *conf, const gchar *group, const gchar *key, gint *value, gint def_value);
void        ngf_conf_get_integer_list (NgfConf *conf, const gchar *group, const gchar *key, gint **value, gint *num_values);

#endif /* NGF_CONF_H */
