/*
 * ngfd - Non-graphic feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation.
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "profile-plugin.h"
#include "session.h"

#include <profiled/libprofile.h>
#include <stdlib.h>

#include <ngf/plugin.h>
#include <ngf/event.h>
#include <ngf/context.h>

#define LOG_CAT                 "profile: "
#define PROFILE_KEY_SUFFIX      ".profile"

#define KEY_VIBRATION_ENABLED   "vibrating.alert.enabled"
#define SILENT_PROFILE          "silent"
#define MEETING_PROFILE         "meeting"

#define TONE_SUFFIX             ".tone"
#define VOLUME_SUFFIX           ".volume"
#define SYSTEM_SUFFIX           ".sound.level"
#define PATTERN_SUFFIX          ".pattern"

#define CLAMP_VALUE(in_v,in_min,in_max) \
    ((in_v) <= (in_min) ? (in_min) : ((in_v) >= (in_max) ? (in_max) : (in_v)))

N_PLUGIN_NAME        ("profile")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_AUTHOR      ("Harri Mahonen <ext-harri.mahonen@nokia.com>")
N_PLUGIN_DESCRIPTION ("Profile support")

typedef struct _ProfileEntry
{
    gchar  *key;
    gchar  *profile;
    gchar  *target;
} ProfileEntry;

static guint       num_system_sound_levels = 0;
static int        *system_sound_levels     = NULL;
static GList      *request_keys            = NULL;
static GHashTable *profile_entries         = NULL;

static void          transform_properties_cb      (NHook *hook,
                                                   void *data,
                                                   void *userdata);
static ProfileEntry* parse_profile_entry          (const char *value);
static void          free_entry                   (ProfileEntry *entry);
static void          find_entries_within_event_cb (const char *key,
                                                   const NValue *value,
                                                   gpointer userdata);
static void          find_profile_entries         (NCore *core);
static void          value_changed_cb             (const char *profile,
                                                   const char *key,
                                                   const char *value,
                                                   const char *type,
                                                   void *userdata);
static void          profile_changed_cb           (const char *profile,
                                                   void *userdata);
static void          query_current_profile        (NCore *core);
static void          query_current_values         (NCore *core);
static gchar*        construct_context_key        (const char *profile,
                                                   const char *key);
static void          update_context_value         (NContext *context,
                                                   const char *profile,
                                                   const char *key,
                                                   const char *value);



static void
transform_properties_cb (NHook *hook, void *data, void *userdata)
{
    (void) hook;
    (void) data;
    (void) userdata;

    NCore        *core        = (NCore*) userdata;
    NContext     *context     = n_core_get_context (core);
    NProplist    *new_props   = NULL;
    NProplist    *props       = NULL;
    GList        *iter        = NULL;
    const char   *match_str   = NULL;
    ProfileEntry *entry       = NULL;
    NValue       *value       = NULL;
    gchar        *context_key = NULL;

    NCoreHookTransformPropertiesData *transform = (NCoreHookTransformPropertiesData*) data;

    N_DEBUG (LOG_CAT "transforming profile values for request '%s'",
        n_request_get_name (transform->request));

    new_props = n_proplist_new ();
    props = (NProplist*) n_request_get_properties (transform->request);
    for (iter = g_list_first (request_keys); iter; iter = g_list_next (iter)) {
        match_str = n_proplist_get_string (props, (gchar*) iter->data);
        if (!match_str)
            continue;

        entry = g_hash_table_lookup (profile_entries, match_str);
        if (!entry)
            continue;

        /* if there already is a target key within the request, then nothing to
           do here. */

        if (n_proplist_has_key (props, entry->target))
            continue;

        context_key = construct_context_key (entry->profile, entry->key);
        value = (NValue*) n_context_get_value (context, context_key);

        if (value) {
            N_DEBUG (LOG_CAT "+ transforming profile key '%s' to target '%s'",
                entry->key, entry->target);
            n_proplist_set (new_props, entry->target, n_value_copy (value));
        }

        g_free (context_key);
    }

    n_proplist_merge (props, new_props);
    n_proplist_free (new_props);

    N_DEBUG (LOG_CAT "new properties:")
    n_proplist_dump (props);
}

static ProfileEntry*
parse_profile_entry (const char *value)
{
    g_assert (value != NULL);

    ProfileEntry  *entry         = NULL;
    gchar        **tokens        = NULL;
    gchar        **source_tokens = NULL;

    /* split the profile key and target key */

    tokens = g_strsplit (value, "=>", 2);
    if (tokens[1] == NULL) {
        N_WARNING (LOG_CAT "profile store key missing for '%s'", value);
        g_strfreev (tokens);
        return NULL;
    }

    g_strstrip (tokens[0]);
    g_strstrip (tokens[1]);

    /* figure out the source profile for the key */

    source_tokens = g_strsplit (tokens[0], "@", 2);

    g_strstrip (source_tokens[0]);
    if (source_tokens[1])
        g_strstrip (source_tokens[1]);

    /* new entry */

    entry = g_new0 (ProfileEntry, 1);
    entry->key     = g_strdup (source_tokens[0]);
    entry->profile = g_strdup (source_tokens[1]);
    entry->target  = g_strdup (tokens[1]);

    g_strfreev (source_tokens);
    g_strfreev (tokens);

    return entry;
}

static void
free_entry (ProfileEntry *entry)
{
    g_free (entry->key);
    g_free (entry->profile);
    g_free (entry->target);
    g_free (entry);
}

static void
find_entries_within_event_cb (const char *key, const NValue *value,
                              gpointer userdata)
{
    (void) key;
    (void) value;
    (void) userdata;

    ProfileEntry *entry     = NULL;
    ProfileEntry *match     = NULL;
    const char   *value_str = NULL;

    if (!g_str_has_suffix (key, PROFILE_KEY_SUFFIX))
        return;

    N_DEBUG (LOG_CAT "possible entry for profile key '%s'", key);

    value_str = n_value_get_string ((NValue*) value);
    entry = parse_profile_entry (value_str);

    if (!entry)
        return;

    match = (ProfileEntry*) g_hash_table_lookup (profile_entries, value_str);
    if (!match) {
        g_hash_table_insert (profile_entries, g_strdup (value_str), entry);
        N_DEBUG (LOG_CAT "new profile entry with key '%s', profile '%s' and target '%s'",
            entry->key, entry->profile, entry->target);

        request_keys = g_list_append (request_keys, g_strdup (key));
    }
    else {
        free_entry (entry);
    }
}

static void
find_profile_entries (NCore *core)
{
    GList     *event_list = NULL;
    GList     *iter       = NULL;
    NEvent    *event      = NULL;
    NProplist *props      = NULL;

    event_list = n_core_get_events (core);
    for (iter = g_list_first (event_list); iter; iter = g_list_next (iter)) {
        event = (NEvent*) iter->data;
        props = (NProplist*) n_event_get_properties (event);

        N_DEBUG (LOG_CAT "searching profile entries from event '%s'",
            n_event_get_name (event));

        n_proplist_foreach (props, find_entries_within_event_cb, NULL);
    }
}

void
profile_plugin_reconnect (NCore *core, DBusConnection *session_bus)
{
    profile_connection_set (session_bus);

    query_current_profile (core);
    query_current_values (core);
}

static gchar*
construct_context_key (const char *profile, const char *key)
{
    const char *profile_str = NULL;
    profile_str = profile ? profile : "current";
    return g_strdup_printf ("profile.%s.%s", profile_str, key);
}

static void
update_context_value (NContext *context, const char *profile, const char *key,
                      const char *value)
{
    gchar  *context_key = NULL;
    NValue *context_val = NULL;
    gint    level       = 0;

    context_key = construct_context_key (profile, key);
    context_val = n_value_new ();

    if (g_str_has_suffix (key, VOLUME_SUFFIX)) {
        n_value_set_int (context_val, profile_parse_int (value));
    }
    else if (g_str_has_suffix (key, SYSTEM_SUFFIX)) {
        level = profile_parse_int (value);
        level = CLAMP_VALUE (level, 0, (gint) num_system_sound_levels-1);
        n_value_set_int (context_val, system_sound_levels[level]);
    }
    else {
        n_value_set_string (context_val, value);
    }

    n_context_set_value (context, context_key, context_val);
    g_free (context_key);
}

static void
value_changed_cb (const char *profile,
                  const char *key,
                  const char *value,
                  const char *type,
                  void *userdata)
{
    (void) type;

    NCore      *core      = (NCore*) userdata;
    NContext   *context   = n_core_get_context (core);
    const char *current   = NULL;

    N_DEBUG (LOG_CAT "value changed for key '%s' ('%s') to '%s'",
        key, profile, value);

    update_context_value (context, profile, key, value);

    /* update current profile value if necessary */

    current = n_value_get_string ((NValue*) n_context_get_value (context,
        "profile.current_profile"));
    if (current && g_str_equal (current, profile))
        update_context_value (context, NULL, key, value);
}

static void
profile_changed_cb (const char *profile,
                    void *userdata)
{
    (void) profile;
    (void) userdata;

    NCore    *core    = (NCore*) userdata;
    NContext *context = n_core_get_context (core);
    NValue   *value   = NULL;

    /* store the current profile to the context */

    value = n_value_new ();
    n_value_set_string (value, profile);
    n_context_set_value (context, "profile.current_profile", value);
    N_DEBUG (LOG_CAT "current profile changed to '%s'", profile);
}

static void
query_current_profile (NCore *core)
{
    NContext   *context = n_core_get_context (core);
    NValue     *value   = NULL;
    char       *profile = NULL;

    profile = profile_get_profile ();

    /* store the current profile to the context */

    value = n_value_new ();
    n_value_set_string (value, profile);
    n_context_set_value (context, "profile.current_profile", value);
    N_DEBUG (LOG_CAT "current profile set to '%s'", profile);

    free (profile);
}

static void
query_current_values (NCore *core)
{
    NContext   *context     = n_core_get_context (core);
    char      **keys        = NULL;
    char      **profiles    = NULL;
    char      **k           = NULL;
    char      **p           = NULL;
    char       *value       = NULL;
    const char *current     = NULL;

    profiles = profile_get_profiles ();
    keys     = profile_get_keys ();
    current  = n_value_get_string ((NValue*) n_context_get_value (context,
        "profile.current_profile"));

    for (p = profiles; *p; ++p) {
        for (k = keys; *k; ++k) {
            value = profile_get_value (*p, *k);
            if (!value)
                continue;

            update_context_value (context, *p, *k, value);
            if (current && g_str_equal (current, *p))
                update_context_value (context, NULL, *k, value);

            free (value);
        }
    }

    profile_free_keys (keys);
    profile_free_profiles (profiles);
}

static void
setup_system_sound_levels (NValue *value)
{
    gchar **split = NULL;
    gchar **iter  = NULL;
    guint   i     = 0;

    if (!value) {
        N_WARNING (LOG_CAT "no system-sound-levels key defined "
                           "in profile.ini!");
        return;
    }

    if (n_value_type (value) != N_VALUE_TYPE_STRING) {
        N_WARNING (LOG_CAT "invalid value type for system sound levels!");
        return;
    }

    split = g_strsplit (n_value_get_string (value), ";", -1);
    for (iter = split; *iter; ++iter)
        ++num_system_sound_levels;

    system_sound_levels = (int*) g_malloc0 (sizeof (int) * num_system_sound_levels);
    for (iter = split, i = 0; *iter; ++iter, ++i) {
        system_sound_levels[i] = atoi (*iter);
        system_sound_levels[i] = CLAMP_VALUE (system_sound_levels[i], 0, 100);
    }

    g_strfreev (split);
}

N_PLUGIN_LOAD (plugin)
{
    NCore     *core   = NULL;
    NProplist *params = NULL;

    profile_entries = g_hash_table_new_full (g_str_hash, g_str_equal,
        g_free, (GDestroyNotify) free_entry);

    /* find all profile key entries within events. */

    core = n_plugin_get_core (plugin);
    find_profile_entries (core);

    /* connect to the transform properties hook. */

    (void) n_core_connect (core, N_CORE_HOOK_TRANSFORM_PROPERTIES,
        0, transform_properties_cb, core);

    /* query the system sound volume levels */

    params = (NProplist*) n_plugin_get_params (plugin);
    setup_system_sound_levels (n_proplist_get (params,
        "system-sound-levels"));

    /* setup the profile client */

    profile_connection_disable_autoconnect ();

    profile_track_add_active_cb  (value_changed_cb, core, NULL);
    profile_track_add_change_cb  (value_changed_cb, core, NULL);
    profile_track_add_profile_cb (profile_changed_cb, core, NULL);

    profile_tracker_init ();

    /* setup the session */

    if (!session_initialize (core))
        return FALSE;

    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    session_shutdown     ();
    profile_tracker_quit ();

    g_free               (system_sound_levels);
    g_hash_table_destroy (profile_entries);
    g_list_foreach       (request_keys, (GFunc) g_free, NULL);
    g_list_free          (request_keys);

    (void) plugin;
}
