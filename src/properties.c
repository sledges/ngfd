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

#include "log.h"
#include "property.h"
#include "properties.h"

GHashTable*
properties_new ()
{
    return g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) property_free);
}

static void
_copy_property_cb (gpointer k, gpointer v, gpointer userdata)
{
    const char *key        = (const char*) k;
    Property   *value      = (Property*) v;
    GHashTable *properties = (GHashTable*) userdata;

    g_hash_table_insert (properties, g_strdup (key), (gpointer) property_copy (value));
}

GHashTable*
properties_copy (GHashTable *source)
{
    GHashTable *target = NULL;

    target = properties_new ();
    g_hash_table_foreach (source, _copy_property_cb, target);
    return target;
}

void
properties_merge (GHashTable *target, GHashTable *source)
{
    GHashTableIter  iter;
    const char     *key   = NULL;
    Property       *value = NULL;

    g_hash_table_iter_init (&iter, source);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value)) {
        g_hash_table_replace (target, g_strdup (key), property_copy (value));
    }
}

void
properties_merge_allowed (GHashTable *target, GHashTable *source, gchar **allowed_keys)
{
    gchar      **allowed = NULL;
    Property    *value   = NULL;

    if (target == NULL || source == NULL || allowed_keys == NULL)
        return;

    for (allowed = allowed_keys; *allowed != NULL; ++allowed) {
        if ((value = (Property*) g_hash_table_lookup (source, *allowed)) != NULL)
            g_hash_table_replace (target, g_strdup (*allowed), property_copy (value));
    }
}

static gchar*
_get_property_str (Property *value)
{
    if (value == NULL)
        return g_strdup ("<value null>");

    switch (property_get_type (value)) {
        case PROPERTY_TYPE_STRING:
            return g_strdup (property_get_string (value));
        case PROPERTY_TYPE_INT:
            return g_strdup_printf ("%d", property_get_int (value));
        case PROPERTY_TYPE_UINT:
            return g_strdup_printf ("%d", property_get_uint (value));
        case PROPERTY_TYPE_BOOLEAN:
            return g_strdup_printf ("%s", property_get_boolean (value) ? "TRUE" : "FALSE");
        default:
            break;
    }

    return NULL;
}

void
properties_dump (GHashTable *properties)
{
    const char *key   = NULL;
    Property   *value = NULL;
    gchar      *str   = NULL;
    GHashTableIter iter;

    g_hash_table_iter_init (&iter, properties);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value)) {
        str = _get_property_str (value);
        LOG_DEBUG ("+ <property %s = %s>", key, str);
        g_free (str);
    }
}

const char*
properties_get_string (GHashTable *source, const char *name)
{
    Property *value = NULL;

    value = (Property*) g_hash_table_lookup (source, name);
    if (value && property_get_type (value) == PROPERTY_TYPE_STRING)
        return property_get_string (value);

    return NULL;
}

gint
properties_get_int (GHashTable *source, const char *name)
{
    Property *value = NULL;

    value = (Property*) g_hash_table_lookup (source, name);
    if (value && property_get_type (value) == PROPERTY_TYPE_INT)
        return property_get_int (value);

    return -1;
}

gboolean
properties_get_bool (GHashTable *source, const char *name)
{
    Property *value = NULL;

    value = (Property*) g_hash_table_lookup (source, name);
    if (value && property_get_type (value) == PROPERTY_TYPE_BOOLEAN)
        return property_get_boolean (value);

    return FALSE;
}