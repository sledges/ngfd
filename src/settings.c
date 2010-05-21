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

#include "log.h"
#include "daemon.h"
#include "property.h"
#include "properties.h"
#include "event-prototype.h"

#define GROUP_GENERAL    "general"
#define GROUP_VIBRATOR   "vibra"
#define GROUP_VOLUME     "volume_pattern"
#define GROUP_DEFINITION "event"
#define GROUP_PROTOTYPE  "prototype"

#define STREAM_RESTORE_ID "module-stream-restore.id"

/** Temporary structure when configuration is being parsed */
typedef struct _SettingsData
{
    Context  *self;
    gchar     **allowed_keys;
} SettingsData;

/**
 * Strip the group type (event, prototype) from the group name.
 *
 * @param group Group name
 * @returns Newly allocated string without the group type or NULL.
 */

static gchar*
_strip_group_type (const char *group)
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

/**
 * Get the name from the group. The name represents the name in the group
 * name without the optional parent inheritance.
 *
 * @param group Group name
 * @returns Newly allocated string with name or NULL if no name.
 *
 * @code
 * name = _parse_group_name ("event my_event@parent_event");
 * g_assert (g_str_equal (name, "my_event");
 * @endcode
 */

static gchar*
_parse_group_name (const char *group)
{
    gchar **split  = NULL;
    gchar  *name   = NULL;
    gchar  *result = NULL;

    name = _strip_group_type (group);
    if (name == NULL)
        return NULL;

    if ((split = g_strsplit (name, "@", 2)) == NULL) {
        g_free (name);
        return NULL;
    }

    result = g_strdup (split[0]);
    g_strfreev (split);
    g_free (name);
    return result;
}

/**
 * Get the parent name of the group. To support simple inheritance, optional
 * parent name can be provided by using the "@" character as a separator.
 *
 * @param group Group name
 * @returns Newly allocated string with parent name or NULL if no parent.
 *
 * @code
 * parent = _parse_group_parent ("event my_event@parent_event");
 * g_assert (g_str_equal (parent, "parent_event");
 * @endcode
 */

static gchar*
_parse_group_parent (const char *group)
{
    gchar **split  = NULL;
    gchar  *parent = NULL;
    gchar  *name   = NULL;

    if (group == NULL)
        return NULL;

    name = _strip_group_type (group);
    if (name == NULL)
        return NULL;

    if ((split = g_strsplit (name, "@", 2)) == NULL) {
        g_free (name);
        return NULL;
    }

    if (split[1] != NULL)
        parent = g_strdup (split[1]);

    g_strfreev (split);
    g_free (name);
    return parent;
}

/**
 * Parse general options.
 *
 * @param data SettingsData
 * @param k GKeyFile
 */

static void
_parse_general (SettingsData *data, GKeyFile *k)
{
    /* Get list of allowed properties that we allow to be overridden through
       the client interface. */

    gchar **allowed_keys = NULL;
    allowed_keys = g_key_file_get_string_list (k, GROUP_GENERAL, "allowed_keys", NULL, NULL);
    if (allowed_keys == NULL)
        return;

    gchar **p = NULL;
    for (p = allowed_keys; *p != NULL; ++p)
        LOG_DEBUG ("<allowed override key: %s>", *p);

    data->allowed_keys = allowed_keys;
}

/**
 * Parse the vibrator patterns defined in the configuration file. For each
 * pattern we will get the IVT file and pattern id and register it to the
 * vibrator backend.
 *
 * @param data SettingsData
 * @param k GKeyFile
 * @post Vibrator patterns registered to the backend.
 */

static void
_parse_vibra_patterns (SettingsData *data, GKeyFile *k)
{
    Context *self = data->self;

    gchar **group_list = NULL;
    gchar **group      = NULL;
    gchar  *filename   = NULL;
    gchar  *name       = NULL;
    gint    pattern_id = 0;

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_VIBRATOR))
            continue;

        name = _parse_group_name (*group);
        if (name == NULL)
            continue;

        filename   = g_key_file_get_string (k, *group, "filename", NULL);
        pattern_id = g_key_file_get_integer (k, *group, "pattern_id", NULL);

        LOG_DEBUG ("<new vibrator pattern: %s (filename=%s, pattern_id=%d)>", name, filename, pattern_id);

        vibrator_register (self->vibrator, name, filename, pattern_id);

        g_free (filename);
        g_free (name);
    }

    g_strfreev (group_list);
}

static void
_parse_volume_patterns (SettingsData *data, GKeyFile *k)
{
    Context *self = data->self;

    gchar    **group_list = NULL;
    gchar    **group      = NULL;
    gchar     *name       = NULL;
    gchar     *pattern    = NULL;
    gboolean   repeat     = FALSE;

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_VOLUME))
            continue;

        name = _parse_group_name (*group);
        if (name == NULL)
            continue;

        pattern = g_key_file_get_string (k, *group, "pattern", NULL);
        repeat  = g_key_file_get_boolean (k, *group, "repeat", NULL);

        LOG_DEBUG ("<new volume pattern: %s (pattern=%s, repeat=%s)>", name, pattern, repeat ? "TRUE" : "FALSE");

        audio_register_controller (self->audio, name, pattern, repeat);

        g_free (pattern);
        g_free (name);
    }

    g_strfreev (group_list);
}

/**
 * Parse event definitions. Event definition contains a reference to
 * long and short prototypes.
 *
 * @param data SettingsData
 * @param k GKeyFile
 * @post New event definition registered to event definitions.
 */

static void
_parse_definitions (SettingsData *data, GKeyFile *k)
{
    Context *self = data->self;

    gchar **group_list = NULL;
    gchar **group      = NULL;
    gchar  *name       = NULL;

    Definition *def = NULL;

    /* For each group that begins with GROUP_DEFINITION, get the values for long and
       short prototypes. */

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_DEFINITION))
            continue;

        name = _parse_group_name (*group);
        if (name == NULL)
            continue;

        def = definition_new ();
        def->long_proto    = g_key_file_get_string (k, *group, "long", NULL);
        def->short_proto   = g_key_file_get_string (k, *group, "short", NULL);
        def->meeting_proto = g_key_file_get_string (k, *group, "meeting", NULL);

        LOG_DEBUG ("<new definition> %s (long=%s, short=%s, meeting=%s)", name, def->long_proto, def->short_proto, def->meeting_proto);
        daemon_register_definition (self, name, def);
    }

    g_strfreev (group_list);
}

/**
 * Iterate the done list and check if the list contains a name.
 *
 * @param done_list GList of strings containing prototype names.
 * @param name Name of prototype to look for.
 * @returns TRUE if found, else FALSE.
 */

static gboolean
_prototype_is_done (GList *done_list, const char *name)
{
    GList *iter = NULL;

    for (iter = g_list_first (done_list); iter; iter = g_list_next (iter)) {
        if (iter->data && g_str_equal ((const char*) iter->data, name))
            return TRUE;
    }

    return FALSE;
}

static void
_add_property_int (EventPrototype *prototype,
                   GKeyFile          *k,
                   const char        *group,
                   const char        *key,
                   gint               def_value,
                   gboolean           set_default)
{
    GError   *error  = NULL;
    gint      result = 0;
    Property *value  = NULL;

    result = g_key_file_get_integer (k, group, key, &error);
    if (error != NULL) {
        if (error->code == G_KEY_FILE_ERROR_INVALID_VALUE)
            LOG_WARNING ("Invalid value for property %s, expected integer. Using default value %d", key, def_value);
        g_error_free (error);

        if (!set_default)
            return;

        result = def_value;
    }

    value = property_new ();
    property_set_int (value, result);

    g_hash_table_insert (prototype->properties, g_strdup (key), value);
}

static void
_add_property_bool (EventPrototype *prototype,
                    GKeyFile          *k,
                    const char        *group,
                    const char        *key,
                    gboolean           def_value,
                    gboolean           set_default)
{
    GError   *error  = NULL;
    gboolean  result = FALSE;
    Property *value  = NULL;

    result = g_key_file_get_boolean (k, group, key, &error);
    if (error != NULL) {
        if (error->code == G_KEY_FILE_ERROR_INVALID_VALUE)
            LOG_WARNING ("Invalid value for property %s, expected boolean. Using default value %s", key, def_value ? "TRUE" : "FALSE");
        g_error_free (error);

        if (!set_default)
            return;

        result = def_value;
    }

    value = property_new ();
    property_set_boolean (value, result);

    g_hash_table_insert (prototype->properties, g_strdup (key), value);
}

static void
_add_property_string (EventPrototype *prototype,
                      GKeyFile          *k,
                      const char        *group,
                      const char        *key,
                      const char        *def_value,
                      gboolean           set_default)
{
    GError   *error  = NULL;
    gchar    *result = NULL;
    Property *value  = NULL;

    result = g_key_file_get_string (k, group, key, &error);
    if (error != NULL) {
        if (error->code == G_KEY_FILE_ERROR_INVALID_VALUE)
            LOG_WARNING ("Invalid value for property %s, expected string. Using default value %s", key, def_value);
        g_error_free (error);

        if (!set_default)
            return;

        result = g_strdup (def_value);
    }

    value = property_new ();
    property_set_string (value, result);

    g_hash_table_insert (prototype->properties, g_strdup (key), value);
    g_free (result);
}

/**
 * Strip a prefix out of the given string.
 *
 * @param str Original string
 * @param prefix Prefix to strip
 * @returns Newly allocated string without prefix.
 */

static gchar*
_strip_prefix (const gchar *str, const gchar *prefix)
{
    if (!g_str_has_prefix (str, prefix))
        return NULL;

    size_t prefix_length = strlen (prefix);
    return g_strdup (str + prefix_length);
}

/**
 * Parse all stream properties and create a Pulseaudio property
 * list out of them.
 *
 * @param prototype EventPrototype
 * @param k GKeyFile
 * @param group Group name
 * @param prefix Prefix to match the properties
 * @param default_role Default audio stream role
 * @post New pa_proplist set to the EventPrototype.
 */

static void
_parse_stream_properties (EventPrototype *prototype,
                          GKeyFile          *k,
                          const char        *group,
                          const char        *prefix,
                          const char        *default_role)
{
    gchar **keys = NULL, **iter = NULL, *stream_prop = NULL, *stream_value = NULL;
    pa_proplist *p = NULL;

    keys = g_key_file_get_keys (k, group, NULL, NULL);

    p = pa_proplist_new ();
    for (iter = keys; *iter; iter++) {
        if ((stream_prop = _strip_prefix (*iter, prefix)) != NULL) {
            stream_value = g_key_file_get_string (k, group, *iter, NULL);

            if (stream_value != NULL)
               pa_proplist_sets (p, stream_prop, stream_value);

            g_free (stream_value);
            g_free (stream_prop);
        }
    }

    g_strfreev (keys);

    /* Setup the default role to the properties if such one
       is defined. */

    if (default_role != NULL)
        pa_proplist_sets (p, STREAM_RESTORE_ID, default_role);

    prototype->stream_properties = p;
}

/**
 * Parse a single prototype. If the prototype has a parent, make sure that
 * we parse it first. Once done, merge the parent's and new prototypes
 * properties and add the prototype to the prototypes_done list to ensure
 * that we don't parse it again.
 *
 * @param self Context
 * @param k GKeyFile
 * @param prototypes_done GList of strings containing parsed prototypes.
 * @param prototypes GHashTable of mapping of prototype name to group.
 * @param name Name of prototype.
 * @post New prototype registered to the daemon.
 */

static void
_parse_single_prototype (SettingsData *data, GKeyFile *k, GList **prototypes_done, GHashTable *prototypes, const char *name)
{
    Context *self = data->self;

    const gchar       *group        = NULL;
    gchar             *parent       = NULL;
    EventPrototype *p            = NULL;
    EventPrototype *copy         = NULL;
    gchar             *default_role = NULL;
    gboolean           set_default  = FALSE;

    if (_prototype_is_done (*prototypes_done, name))
        return;

    if ((group = g_hash_table_lookup (prototypes, name)) == NULL)
        return;

    if ((parent = _parse_group_parent (group)) != NULL)
        _parse_single_prototype (data, k, prototypes_done, prototypes, parent);

    if (name == NULL)
        return;

    /* set_default flag is set if the prototype does not have a parent element (ie. it is a parent
       element by itself). */

    set_default = (parent == NULL) ? TRUE : FALSE;

    p = event_prototype_new ();

    /* Begin parsing of prototype */

    default_role = g_strdup_printf ("x-maemo-%s", name);

    _add_property_int        (p, k, group, "max_length", FALSE, set_default);

    _add_property_bool       (p, k, group, "audio_enabled", FALSE, set_default);
    _add_property_bool       (p, k, group, "audio_repeat", FALSE, set_default);
    _add_property_int        (p, k, group, "audio_max_repeats", 0, set_default);
    _add_property_string     (p, k, group, "audio", NULL, set_default);
    _add_property_string     (p, k, group, "audio_tone_profile", NULL, set_default);
    _add_property_bool       (p, k, group, "audio_silent", FALSE, set_default);

    _add_property_string     (p, k, group, "audio_fallback_filename", NULL, set_default);
    _add_property_string     (p, k, group, "audio_fallback_profile", NULL, set_default);

    _add_property_int        (p, k, group, "audio_volume_value", -1, set_default);
    _add_property_string     (p, k, group, "audio_volume_profile", NULL, set_default);
    _add_property_string     (p, k, group, "audio_volume_pattern", NULL, set_default);
    _add_property_string     (p, k, group, "audio_stream_role", default_role, TRUE);

    _add_property_bool       (p, k, group, "audio_tonegen_enabled", FALSE, set_default);
    _add_property_int        (p, k, group, "audio_tonegen_pattern", -1, set_default);

    _add_property_bool       (p, k, group, "vibra_enabled", FALSE, set_default);
    _add_property_bool       (p, k, group, "vibrator_custom_patterns", FALSE, set_default);
    _add_property_string     (p, k, group, "vibra", NULL, set_default);

    _add_property_bool       (p, k, group, "led_enabled", FALSE, set_default);
    _add_property_string     (p, k, group, "led", NULL, set_default);

    _add_property_bool       (p, k, group, "backlight_enabled", FALSE, set_default);

    _add_property_bool       (p, k, group, "disallow_override", FALSE, set_default);

    g_free (default_role);

    /* Parse the stream properties (all properties beginning with "stream.") */
    _parse_stream_properties (p, k, group, "stream.", properties_get_string (p->properties, "audio_stream_role"));

    /* If we have a parent prototype, make a copy of it and merge our new prototype
       to the copy. */

    if (parent != NULL) {
        copy = event_prototype_copy (daemon_get_prototype (self, parent));

        if (copy != NULL) {
            event_prototype_merge (copy, p);
            event_prototype_free (p);
            p = copy;
        }
    }

    /* Make a copy of the list of allowed keys for this prototype. */
    p->allowed_keys = g_strdupv (data->allowed_keys);

    /* We're done, let's register the new prototype. */
    daemon_register_prototype (self, name, p);

    LOG_DEBUG ("<new prototype %s>", name);
    event_prototype_dump (p);

    *prototypes_done = g_list_append (*prototypes_done, g_strdup (name));
    g_free (parent);
}

/**
 * Parse all prototypes. Get a list of available groups and make a mapping
 * of prototype name to group name. Once done, iterate through the list
 * and parse every prototype.
 *
 * @param self Context
 * @param k GKeyFile
 * @post All available and valid prototypes registered to daemon.
 */

static void
_parse_prototypes (SettingsData *data, GKeyFile *k)
{
    gchar         **group_list = NULL;
    gchar         **group      = NULL;
    gchar          *name       = NULL;
    GHashTable *prototypes = NULL;

    prototypes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    /* Get the available prototypes and map name to group */

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_PROTOTYPE))
            continue;

        if ((name = _parse_group_name (*group)) != NULL)
            g_hash_table_insert (prototypes, name, g_strdup (*group));
    }

    g_strfreev (group_list);

    /* For each entry in the map of prototypes ... */

    GHashTableIter iter;
    gchar *key, *value;
    GList *prototypes_done = NULL;

    g_hash_table_iter_init (&iter, prototypes);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value))
        _parse_single_prototype (data, k, &prototypes_done, prototypes, key);

    g_list_foreach (prototypes_done, (GFunc) g_free, NULL);
    g_list_free (prototypes_done);

    g_hash_table_destroy (prototypes);
}

gboolean
daemon_settings_load (Context *self)
{
    static const char *conf_files[] = { "/etc/ngf/ngf.ini", "./ngf.ini", NULL };

    SettingsData  *data     = NULL;
    GKeyFile      *key_file = NULL;
    const char   **filename = NULL;
    gboolean       success  = FALSE;

    if ((key_file = g_key_file_new ()) == NULL)
        return FALSE;

    for (filename = conf_files; *filename != NULL; ++filename) {
        if (g_key_file_load_from_file (key_file, *filename, G_KEY_FILE_NONE, NULL)) {
            success = TRUE;
            break;
        }
    }

    if (!success)
        return FALSE;

    data = g_new0 (SettingsData, 1);
    data->self = self;

    _parse_general            (data, key_file);
    _parse_vibra_patterns     (data, key_file);
    _parse_volume_patterns    (data, key_file);
    _parse_definitions        (data, key_file);
    _parse_prototypes         (data, key_file);

    g_strfreev (data->allowed_keys);
    g_free (data);

    return TRUE;
}
