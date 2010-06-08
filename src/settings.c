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

#include <stdlib.h>
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

#define ARRAY_SIZE(x) (sizeof (x) / sizeof (x[0]))

enum
{
    KEY_ENTRY_TYPE_STRING,
    KEY_ENTRY_TYPE_INT,
    KEY_ENTRY_TYPE_BOOL
};

typedef struct _KeyEntry
{
    guint       type;
    const char *key;
    gint        def_int;
    const char *def_str;
} KeyEntry;

typedef struct _SettingsData
{
    Context    *context;
    GHashTable *groups;
    GHashTable *events;
} SettingsData;

static KeyEntry event_entries[] = {
    /* general */
    { KEY_ENTRY_TYPE_INT   , "max_timeout"          , 0    , NULL },
    { KEY_ENTRY_TYPE_BOOL  , "allow_custom"         , FALSE, NULL },
    { KEY_ENTRY_TYPE_INT   , "dummy"                , 0    , NULL },

    /* sound */
    { KEY_ENTRY_TYPE_BOOL  , "audio_enabled"        , FALSE, NULL },
    { KEY_ENTRY_TYPE_BOOL  , "audio_repeat"         , FALSE, NULL },
    { KEY_ENTRY_TYPE_INT   , "audio_max_repeats"    , 0    , NULL },
    { KEY_ENTRY_TYPE_STRING, "sound"                , 0    , NULL },
    { KEY_ENTRY_TYPE_BOOL  , "silent_enabled"       , FALSE, NULL },
    { KEY_ENTRY_TYPE_STRING, "volume"               , 0    , NULL },
    { KEY_ENTRY_TYPE_STRING, "event_id"             , 0    , NULL },

    /* tonegen */
    { KEY_ENTRY_TYPE_INT   , "audio_tonegen_pattern", -1   , NULL },

    /* vibration */
    { KEY_ENTRY_TYPE_BOOL  , "vibration_enabled"    , FALSE, NULL },
    { KEY_ENTRY_TYPE_BOOL  , "lookup_pattern"       , FALSE, NULL },
    { KEY_ENTRY_TYPE_STRING, "vibration"            , FALSE, NULL },

    /* led */
    { KEY_ENTRY_TYPE_BOOL  , "led_enabled"          , FALSE, NULL },
    { KEY_ENTRY_TYPE_STRING, "led_pattern"          , 0    , NULL },

    /* backlight */
    { KEY_ENTRY_TYPE_BOOL  , "backlight_enabled"    , FALSE, NULL },
    { KEY_ENTRY_TYPE_BOOL  , "unlock_tklock"        , FALSE, NULL },
};



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
_add_property_int (GHashTable *target,
                   GKeyFile   *k,
                   const char *group,
                   const char *key,
                   gint        def_value,
                   gboolean    set_default)
{
    GError   *error  = NULL;
    gint      result = 0;
    Property *value  = NULL;

    result = g_key_file_get_integer (k, group, key, &error);
    if (error != NULL) {
        if (error->code == G_KEY_FILE_ERROR_INVALID_VALUE)
            LOG_WARNING ("Invalid value for property %s, expected integer. Using default value %d", key, def_value);
        g_error_free (error);
        result = def_value;

        if (!set_default)
            return;
    }

    value = property_new ();
    property_set_int (value, result);

    g_hash_table_insert (target, g_strdup (key), value);
}

static void
_add_property_bool (GHashTable *target,
                    GKeyFile   *k,
                    const char *group,
                    const char *key,
                    gboolean    def_value,
                    gboolean    set_default)
{
    GError   *error  = NULL;
    gboolean  result = FALSE;
    Property *value  = NULL;

    result = g_key_file_get_boolean (k, group, key, &error);
    if (error != NULL) {
        if (error->code == G_KEY_FILE_ERROR_INVALID_VALUE)
            LOG_WARNING ("Invalid value for property %s, expected boolean. Using default value %s", key, def_value ? "TRUE" : "FALSE");
        g_error_free (error);
        result = def_value;

        if (!set_default)
            return;
    }

    value = property_new ();
    property_set_boolean (value, result);

    g_hash_table_insert (target, g_strdup (key), value);
}

static void
_add_property_string (GHashTable *target,
                      GKeyFile   *k,
                      const char *group,
                      const char *key,
                      const char *def_value,
                      gboolean    set_default)
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

    g_hash_table_insert (target, g_strdup (key), value);
    g_free (result);
}

static gchar*
_strip_prefix (const gchar *str, const gchar *prefix)
{
    if (!g_str_has_prefix (str, prefix))
        return NULL;

    size_t prefix_length = strlen (prefix);
    return g_strdup (str + prefix_length);
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

    if (g_str_has_prefix (str, "profile:")) {
        stripped = _strip_prefix (str, "profile:");

        pattern = vibration_pattern_new ();
        pattern->type     = VIBRATION_PATTERN_TYPE_PROFILE;

        if (!_parse_profile_key (stripped, &pattern->profile, &pattern->key)) {
            g_free (stripped);
            return NULL;
        }

        g_free (stripped);
    }
    else if (g_str_has_prefix (str, "filename:")) {
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

static void
_parse_single_event (SettingsData *data, GKeyFile *k, GList **events_done, const char *name)
{
    const gchar *group      = NULL;
    gchar       *parent     = NULL;
    KeyEntry    *entry      = NULL;
    gboolean     is_base    = FALSE;
    GHashTable  *properties = NULL;
    GHashTable  *copy       = NULL;
    guint        i          = 0;

    if (_event_is_done (*events_done, name))
        return;

    if ((group = g_hash_table_lookup (data->groups, name)) == NULL)
        return;

    if ((parent = _parse_group_parent (group)) != NULL)
        _parse_single_event (data, k, events_done, parent);

    if (name == NULL)
        return;

    properties = properties_new ();

    for (i = 0; i < ARRAY_SIZE(event_entries); ++i) {
        entry = &event_entries[i];

        is_base = (parent == NULL) ? TRUE : FALSE;
        switch (entry->type) {
            case KEY_ENTRY_TYPE_STRING:
                _add_property_string (properties, k, group, entry->key, entry->def_str, is_base);
                break;
            case KEY_ENTRY_TYPE_INT:
                _add_property_int (properties, k, group, entry->key, entry->def_int, is_base);
                break;
            case KEY_ENTRY_TYPE_BOOL:
                _add_property_bool (properties, k, group, entry->key, entry->def_int, is_base);
                break;
            default:
                break;
        }
    }

    /* if a parent was defined, merge */
    if (parent != NULL) {
        copy = properties_copy (g_hash_table_lookup (data->events, parent));
        properties_merge (copy, properties);
        g_hash_table_destroy (properties);
        properties = copy;
    }

    g_hash_table_insert (data->events, g_strdup (name), properties);
    *events_done = g_list_append (*events_done, g_strdup (name));
    g_free (parent);
}

static void
finalize_event (SettingsData *data, const char *name, GHashTable *properties)
{
    Context *context = data->context;
    Event   *event   = event_new ();

    event->audio_enabled          = properties_get_bool (properties, "audio_enabled");
    event->vibration_enabled      = properties_get_bool (properties, "vibration_enabled");
    event->leds_enabled           = properties_get_bool (properties, "led_enabled");
    event->backlight_enabled      = properties_get_bool (properties, "backlight_enabled");
    event->unlock_tklock          = properties_get_bool (properties, "unlock_tklock");

    event->allow_custom           = properties_get_bool (properties, "allow_custom");
    event->max_timeout            = properties_get_int  (properties, "max_timeout");
    event->lookup_pattern         = properties_get_bool (properties, "lookup_pattern");
    event->silent_enabled         = properties_get_bool (properties, "silent_enabled");
    event->event_id               = g_strdup (properties_get_string (properties, "event_id"));

    event->tone_generator_enabled = properties_get_bool (properties, "audio_tonegen_enabled");
    event->tone_generator_pattern = properties_get_int (properties, "audio_tonegen_pattern");

    event->repeat                 = properties_get_bool (properties, "audio_repeat");
    event->num_repeats            = properties_get_int (properties, "audio_max_repeats");
    event->led_pattern            = g_strdup (properties_get_string (properties, "led_pattern"));

    event->sounds                 = _create_sound_paths (context, properties_get_string (properties, "sound"));
    event->volume                 = _create_volume      (context, properties_get_string (properties, "volume"));
    event->patterns               = _create_patterns    (context, properties_get_string (properties, "vibration"));

    g_hash_table_replace (context->events, g_strdup (name), event);
}

static void
_parse_events (SettingsData *data, GKeyFile *k)
{
    gchar      **group_list = NULL;
    gchar      **group      = NULL;
    gchar       *name       = NULL;
    GHashTable  *evtdata    = NULL;

    data->events = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_hash_table_destroy);
    data->groups = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    /* Get the available events and map name to group */

    group_list = g_key_file_get_groups (k, NULL);
    for (group = group_list; *group != NULL; ++group) {
        if (!g_str_has_prefix (*group, GROUP_EVENT))
            continue;

        if ((name = _parse_group_name (*group)) != NULL)
            g_hash_table_insert (data->groups, name, g_strdup (*group));
    }

    g_strfreev (group_list);

    /* For each entry in the map of events ... */

    GHashTableIter iter;
    gchar *key, *value;
    GList *events_done = NULL;

    g_hash_table_iter_init (&iter, data->groups);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value))
        _parse_single_event (data, k, &events_done, key);

    g_list_foreach (events_done, (GFunc) g_free, NULL);
    g_list_free (events_done);

    g_hash_table_iter_init (&iter, data->events);
    while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &evtdata))
        finalize_event (data, key, evtdata);

    g_hash_table_destroy (data->groups);
    g_hash_table_destroy (data->events);
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
