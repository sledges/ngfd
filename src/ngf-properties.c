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

#include "ngf-log.h"
#include "ngf-value.h"
#include "ngf-properties.h"

GHashTable*
ngf_properties_new ()
{
    return g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) ngf_value_free);
}

static void
_copy_property_cb (gpointer k, gpointer v, gpointer userdata)
{
    const char *key        = (const char*) k;
    NgfValue   *value      = (const char*) v;
    GHashTable *properties = (GHashTable*) userdata;

    g_hash_table_insert (properties, g_strdup (key), (gpointer) ngf_value_copy (value));
}

GHashTable*
ngf_properties_copy (GHashTable *source)
{
    GHashTable *target = NULL;

    target = ngf_properties_new ();
    g_hash_table_foreach (source, _copy_property_cb, target);
    return target;
}

void
ngf_properties_merge (GHashTable *target, GHashTable *source)
{
    GHashTableIter  iter;
    const char     *key   = NULL;
    NgfValue       *value = NULL;

    g_hash_table_iter_init (&iter, source);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value)) {
        g_hash_table_replace (target, g_strdup (key), ngf_value_copy (value));
    }
}

void
ngf_properties_merge_allowed (GHashTable *target, GHashTable *source, gchar **allowed_keys)
{
    gchar      **allowed = NULL;
    const char  *key     = NULL;
    NgfValue    *value   = NULL;

    if (target == NULL || source == NULL || allowed_keys == NULL)
        return;

    for (allowed = allowed_keys; *allowed != NULL; ++allowed) {
        if ((value = (NgfValue*) g_hash_table_lookup (source, *allowed)) != NULL)
            g_hash_table_replace (target, g_strdup (key), ngf_value_copy (value));
    }
}

static gchar*
_get_value_str (NgfValue *value)
{
    if (value == NULL)
        return g_strdup ("<value null>");

    switch (ngf_value_get_type (value)) {
        case NGF_VALUE_STRING:
            return g_strdup (ngf_value_get_string (value));
        case NGF_VALUE_INT:
            return g_strdup_printf ("%d", ngf_value_get_int (value));
        case NGF_VALUE_UINT:
            return g_strdup_printf ("%d", ngf_value_get_uint (value));
        case NGF_VALUE_BOOLEAN:
            return g_strdup_printf ("%s", ngf_value_get_boolean (value) ? "TRUE" : "FALSE");
        default:
            break;
    }

    return NULL;
}

void
ngf_properties_dump (GHashTable *properties)
{
    const char *key   = NULL;
    NgfValue   *value = NULL;
    gchar      *str   = NULL;
    GHashTableIter iter;

    g_hash_table_iter_init (&iter, properties);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value)) {
        str = _get_value_str (value);
        LOG_DEBUG ("+ <property %s = %s>", key, str);
        g_free (str);
    }
}

const char*
ngf_properties_get_string (GHashTable *source, const char *name)
{
    NgfValue *value = NULL;

    value = (NgfValue*) g_hash_table_lookup (source, name);
    if (value && ngf_value_get_type (value) == NGF_VALUE_STRING)
        return ngf_value_get_string (value);

    return NULL;
}

gint
ngf_properties_get_int (GHashTable *source, const char *name)
{
    NgfValue *value = NULL;

    value = (NgfValue*) g_hash_table_lookup (source, name);
    if (value && ngf_value_get_type (value) == NGF_VALUE_INT)
        return ngf_value_get_int (value);

    return -1;
}

gboolean
ngf_properties_get_bool (GHashTable *source, const char *name)
{
    NgfValue *value = NULL;

    value = (NgfValue*) g_hash_table_lookup (source, name);
    if (value && ngf_value_get_type (value) == NGF_VALUE_BOOLEAN)
        return ngf_value_get_boolean (value);

    return FALSE;
}
