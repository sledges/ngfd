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
#include "property.h"
#include "properties.h"
#include "controller.h"
#include "event.h"
#include "context.h"

#define GROUP_GENERAL    "general"
#define GROUP_VIBRATOR   "vibra"
#define GROUP_VOLUME     "volume_pattern"
#define GROUP_DEFINITION "definition"
#define GROUP_EVENT      "event"

#define STREAM_RESTORE_ID "module-stream-restore.id"

typedef struct _SettingsData
{
    Context *context;
} SettingsData;

/**
 * Strip the group type (event, event) from the group name.
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

static void
_parse_general (SettingsData *data, GKeyFile *k)
{
    (void) data;
    (void) k;
}

static void
_parse_definitions (SettingsData *data, GKeyFile *k)
{
    Context *context = data->context;

    gchar **group_list = NULL;
    gchar **group      = NULL;
    gchar  *name       = NULL;

    Definition *def = NULL;

    /* For each group that begins with GROUP_DEFINITION, get the values for long and
       short events. */

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_DEFINITION))
            continue;

        name = _parse_group_name (*group);
        if (name == NULL)
            continue;

        def = definition_new ();
        def->long_event    = g_key_file_get_string (k, *group, "long", NULL);
        def->short_event   = g_key_file_get_string (k, *group, "short", NULL);
        def->meeting_event = g_key_file_get_string (k, *group, "meeting", NULL);

        LOG_DEBUG ("<new definition> %s (long=%s, short=%s, meeting=%s)", name, def->long_event, def->short_event, def->meeting_event);
        g_hash_table_replace (context->definitions, g_strdup (name), def);
    }

    g_strfreev (group_list);
}

static gboolean
_event_is_done (GList *done_list, const char *name)
{
    GList *iter = NULL;

    for (iter = g_list_first (done_list); iter; iter = g_list_next (iter)) {
        if (iter->data && g_str_equal ((const char*) iter->data, name))
            return TRUE;
    }

    return FALSE;
}

static void
_add_property_int (Event *event,
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

    g_hash_table_insert (event->properties, g_strdup (key), value);
}

static void
_add_property_bool (Event *event,
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

    g_hash_table_insert (event->properties, g_strdup (key), value);
}

static void
_add_property_string (Event *event,
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

    g_hash_table_insert (event->properties, g_strdup (key), value);
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
 * @param event Event
 * @param k GKeyFile
 * @param group Group name
 * @param prefix Prefix to match the properties
 * @param default_role Default audio stream role
 * @post New pa_proplist set to the Event.
 */

static void
_parse_stream_properties (Event *event,
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

    event->stream_properties = p;
}

static gboolean
_parse_profile_key (const char *key, gchar **out_profile, gchar **out_key)
{
    gchar **split = NULL;
    gboolean ret = FALSE;

    if (key == NULL)
        return FALSE;

    if ((split = g_strsplit (key, "@", 2)) != NULL) {
        if (split[0] == NULL || g_str_equal (split[0], "")) {
            ret = FALSE;
            goto done;
        }

        *out_key = g_strdup (split[0]);
        *out_profile = g_strdup (split[1]);
        ret = TRUE;
    }

done:
    g_strfreev (split);
    return ret;
}

static SoundPath*
_parse_sound_path (Context *context, const gchar *str)
{
    SoundPath *sound_path = NULL;
    gchar *stripped = NULL;

    if (str == NULL)
        return NULL;

    if (g_str_has_prefix (str, "profile:")) {
        stripped = _strip_prefix (str, "profile:");

        sound_path       = sound_path_new ();
        sound_path->type = SOUND_PATH_TYPE_PROFILE;

        if (!_parse_profile_key (stripped, &sound_path->profile, &sound_path->key)) {
            g_free (stripped);
            return NULL;
        }

        g_free (stripped);
    }
    else if (g_str_has_prefix (str, "filename:")) {
        stripped = _strip_prefix (str, "filename:");

        sound_path           = sound_path_new ();
        sound_path->type     = SOUND_PATH_TYPE_FILENAME;
        sound_path->filename = stripped;
    }

    return context_add_sound_path (context, sound_path);
}

static GList*
_create_sound_paths (Context *context, const gchar *str)
{
    GList      *result = NULL;
    SoundPath  *sound_path = NULL;
    gchar     **sounds = NULL, **s = NULL;

    if (str == NULL)
        return NULL;

    if ((sounds = g_strsplit (str, ";", -1)) == NULL)
        return NULL;

    for (s = sounds; *s; ++s) {
        if ((sound_path = _parse_sound_path (context, *s)) != NULL)
            result = g_list_append (result, sound_path);
    }

    g_strfreev (sounds);
    return result;
}

static Volume*
_create_volume (Context *context, const gchar *str)
{
    Volume *volume = NULL;
    gchar *stripped = NULL;

    if (str == NULL)
        return NULL;

    if (g_str_has_prefix (str, "profile:")) {
        stripped = _strip_prefix (str, "profile:");

        volume       = volume_new ();
        volume->type = VOLUME_TYPE_PROFILE;

        if (!_parse_profile_key (stripped, &volume->profile, &volume->key)) {
            g_free (stripped);
            return NULL;
        }

        g_free (stripped);
    }
    else if (g_str_has_prefix (str, "fixed:")) {
        stripped = _strip_prefix (str, "fixed:");

        volume        = volume_new ();
        volume->type  = VOLUME_TYPE_FIXED;
        volume->level = atoi (stripped);

        g_free (stripped);
    }
    else if (g_str_has_prefix (str, "controller:")) {
        stripped = _strip_prefix (str, "controller:");

        volume             = volume_new ();
        volume->type       = VOLUME_TYPE_CONTROLLER;
        volume->controller = controller_new_from_string (stripped);

        if (volume->controller == NULL) {
            volume_free (volume);
            g_free (stripped);
            return NULL;
        }

        g_free (stripped);
    }

    return context_add_volume (context, volume);
}

static VibrationPattern*
_parse_pattern (Context *context, const gchar *str)
{
    VibrationPattern *pattern = NULL;
    gchar *stripped = NULL;

    if (str == NULL)
        return NULL;

    if (g_str_has_prefix (str, "filename:")) {
        stripped = _strip_prefix (str, "filename:");

        pattern = vibration_pattern_new ();
        pattern->type     = VIBRATION_PATTERN_TYPE_FILENAME;
        pattern->filename = stripped;
    }
    else if (g_str_has_prefix (str, "internal:")) {
        stripped = _strip_prefix (str, "internal:");

        pattern = vibration_pattern_new ();
        pattern->type    = VIBRATION_PATTERN_TYPE_INTERNAL;
        pattern->pattern = atoi (stripped);

        g_free (stripped);
    }

    return context_add_pattern (context, pattern);
}

static GList*
_create_patterns (Context *context, const gchar *str)
{
    GList             *result = NULL;
    VibrationPattern  *pattern = NULL;
    gchar            **patterns = NULL, **i = NULL;

    if (str == NULL)
        return NULL;

    if ((patterns = g_strsplit (str, ";", -1)) == NULL) {
        return NULL;
    }

    for (i = patterns; *i; ++i) {
        if ((pattern = _parse_pattern (context, *i)) != NULL)
            result = g_list_append (result, pattern);
    }

    g_strfreev (patterns);
    return result;
}

/**
 * Parse a single event. If the event has a parent, make sure that
 * we parse it first. Once done, merge the parent's and new events
 * properties and add the event to the events_done list to ensure
 * that we don't parse it again.
 *
 * @param context Context
 * @param k GKeyFile
 * @param events_done GList of strings containing parsed events.
 * @param events GHashTable of mapping of event name to group.
 * @param name Name of event.
 * @post New event registered to the daemon.
 */

static void
_dump_sound_path (gpointer data, gpointer userdata)
{
    SoundPath *sound_path = (SoundPath*) data;

    LOG_DEBUG ("sound_path <type=%d, filename=%s, key=%s, profile=%s>\n",
        sound_path->type, sound_path->filename, sound_path->key, sound_path->profile);
}

static void
_dump_vibration (gpointer data, gpointer userdata)
{
    VibrationPattern *pattern = (VibrationPattern*) data;
    
    LOG_DEBUG ("vibration_pattern <type=%d, filename=%s, pattern=%d>\n",
            pattern->type, pattern->filename, pattern->pattern);
}

static void
_parse_single_event (SettingsData *data, GKeyFile *k, GList **events_done, GHashTable *events, const char *name)
{
    Context *context = data->context;

    const gchar       *group        = NULL;
    gchar             *parent       = NULL;
    Event             *p            = NULL;
    Event             *copy         = NULL;
    gchar             *default_role = NULL;
    gboolean           set_default  = FALSE;

    if (_event_is_done (*events_done, name))
        return;

    if ((group = g_hash_table_lookup (events, name)) == NULL)
        return;

    if ((parent = _parse_group_parent (group)) != NULL)
        _parse_single_event (data, k, events_done, events, parent);

    if (name == NULL)
        return;

    /* set_default flag is set if the event does not have a parent element (ie. it is a parent
       element by itcontext). */

    set_default = (parent == NULL) ? TRUE : FALSE;

    p = event_new ();

    /* Begin parsing of event */

    default_role = g_strdup_printf ("x-maemo-%s", name);

    _add_property_int        (p, k, group, "max_timeout", FALSE, set_default);
    _add_property_bool       (p, k, group, "audio_enabled", FALSE, set_default);
    _add_property_bool       (p, k, group, "audio_repeat", FALSE, set_default);
    _add_property_int        (p, k, group, "audio_max_repeats", 0, set_default);
    _add_property_string     (p, k, group, "sound", NULL, set_default);
    _add_property_bool       (p, k, group, "silent_enabled", FALSE, set_default);
    _add_property_string     (p, k, group, "volume", NULL, set_default);
    _add_property_string     (p, k, group, "audio_stream_role", default_role, TRUE);
    _add_property_bool       (p, k, group, "audio_tonegen_enabled", FALSE, set_default);
    _add_property_int        (p, k, group, "audio_tonegen_pattern", -1, set_default);
    _add_property_bool       (p, k, group, "vibration_enabled", FALSE, set_default);
    _add_property_bool       (p, k, group, "lookup_pattern", FALSE, set_default);
    _add_property_string     (p, k, group, "vibration", NULL, set_default);
    _add_property_bool       (p, k, group, "led_enabled", FALSE, set_default);
    _add_property_string     (p, k, group, "led", NULL, set_default);
    _add_property_bool       (p, k, group, "backlight_enabled", FALSE, set_default);
    _add_property_bool       (p, k, group, "allow_custom", FALSE, set_default);
    _add_property_bool       (p, k, group, "unlock_tklock", FALSE, set_default);

    g_free (default_role);

    /* Parse the stream properties (all properties beginning with "stream.") */
    _parse_stream_properties (p, k, group, "stream.", properties_get_string (p->properties, "audio_stream_role"));

    /* If we have a parent event, make a copy of it and merge our new event
       to the copy. */

    if (parent != NULL) {
        copy = event_copy (g_hash_table_lookup (context->events, parent));

        if (copy != NULL) {
            event_merge (copy, p);
            event_free (p);
            p = copy;
        }
    }

    /* translate the event */

    p->audio_enabled     = properties_get_bool (p->properties, "audio_enabled");
    p->vibration_enabled = properties_get_bool (p->properties, "vibration_enabled");
    p->leds_enabled      = properties_get_bool (p->properties, "led_enabled");
    p->backlight_enabled = properties_get_bool (p->properties, "backlight_enabled");
    p->unlock_tklock     = properties_get_bool (p->properties, "unlock_tklock");

    p->allow_custom      = properties_get_bool (p->properties, "allow_custom");
    p->max_timeout       = properties_get_int  (p->properties, "max_timeout");
    p->lookup_pattern    = properties_get_bool (p->properties, "lookup_pattern");
    p->silent_enabled    = properties_get_bool (p->properties, "silent_enabled");

    p->tone_generator_enabled = properties_get_bool (p->properties, "audio_tonegen_enabled");
    p->tone_generator_pattern = properties_get_int (p->properties, "audio_tonegen_pattern");

    p->repeat            = properties_get_bool (p->properties, "audio_repeat");
    p->num_repeats       = properties_get_int (p->properties, "audio_max_repeats");
    p->stream_role       = g_strdup (properties_get_string (p->properties, "audio_stream_role"));
    p->led_pattern       = g_strdup (properties_get_string (p->properties, "led_pattern"));

    p->sounds   = _create_sound_paths (context, properties_get_string (p->properties, "sound"));
    p->volume   = _create_volume      (context, properties_get_string (p->properties, "volume"));
    p->patterns = _create_patterns    (context, properties_get_string (p->properties, "vibration"));

    g_list_foreach (p->sounds, _dump_sound_path, NULL);
    g_list_foreach (p->patterns, _dump_vibration, NULL);

    /* We're done, let's register the new event. */
    g_hash_table_replace (context->events, g_strdup (name), p);

    LOG_DEBUG ("<new event %s>", name);

    *events_done = g_list_append (*events_done, g_strdup (name));
    g_free (parent);
}

/**
 * Parse all events. Get a list of available groups and make a mapping
 * of event name to group name. Once done, iterate through the list
 * and parse every event.
 *
 * @param context Context
 * @param k GKeyFile
 * @post All available and valid events registered to daemon.
 */

static void
_parse_events (SettingsData *data, GKeyFile *k)
{
    gchar         **group_list = NULL;
    gchar         **group      = NULL;
    gchar          *name       = NULL;
    GHashTable *events = NULL;

    events = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    /* Get the available events and map name to group */

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_EVENT))
            continue;

        if ((name = _parse_group_name (*group)) != NULL)
            g_hash_table_insert (events, name, g_strdup (*group));
    }

    g_strfreev (group_list);

    /* For each entry in the map of events ... */

    GHashTableIter iter;
    gchar *key, *value;
    GList *events_done = NULL;

    g_hash_table_iter_init (&iter, events);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value))
        _parse_single_event (data, k, &events_done, events, key);

    g_list_foreach (events_done, (GFunc) g_free, NULL);
    g_list_free (events_done);

    g_hash_table_destroy (events);
}

gboolean
load_settings (Context *context)
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
    data->context = context;

    _parse_general     (data, key_file);
    _parse_definitions (data, key_file);
    _parse_events      (data, key_file);

    g_free (data);

    return TRUE;
}
