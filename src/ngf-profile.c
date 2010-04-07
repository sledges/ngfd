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
#include <profiled/libprofile.h>

#include "ngf-profile.h"

#define VIBRATION_ENABLED_KEY "vibrating.alert.enabled"

typedef struct _ProfileEntry
{
    gchar       *profile_name;
    GHashTable  *values;
} ProfileEntry;

struct _NgfProfile
{
    gchar    *current_profile;
    gboolean  is_silent;
    gboolean  is_vibra_enabled;
    GList    *profiles;
};

static void
_profile_entry_free (ProfileEntry *entry)
{
    if (entry->values) {
        g_hash_table_destroy (entry->values);
        entry->values = NULL;
    }

    g_free (entry->profile_name);
    entry->profile_name = NULL;

    g_free (entry);
}

static ProfileEntry*
_profile_entry_new (const char *profile)
{
    ProfileEntry *entry = NULL;

    if (profile == NULL)
        return NULL;

    if ((entry = g_new0 (ProfileEntry, 1)) == NULL)
        return NULL;

    entry->profile_name = g_strdup (profile);
    entry->values = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    return entry;
}

static ProfileEntry*
_profile_entry_find (NgfProfile *self, const char *profile)
{
    GList *iter = NULL;
    ProfileEntry *entry = NULL;

    for (iter = g_list_first (self->profiles); iter; iter = g_list_next (iter)) {
        entry = (ProfileEntry*) iter->data;
        if (entry && g_str_equal (entry->profile_name, profile))
            return entry;
    }

    return NULL;
}

static void
_profile_query_values (NgfProfile *self, char **keys, const char *profile)
{
    char **k = NULL, *value = NULL;
    ProfileEntry *entry = NULL;

    entry = _profile_entry_new (profile);
    self->profiles = g_list_append (self->profiles, entry);

    for (k = keys; *k; k++) {
        value = profile_get_value (profile, *k);
        if (value) {
            g_hash_table_replace (entry->values, g_strdup (*k), g_strdup (value));
            free (value);
        }
    }
}

static void
_profile_setup (NgfProfile *self)
{
    char **profiles = NULL, **keys = NULL, **p = NULL;

    /* Get the initial current profile */

    self->current_profile = g_strdup (profile_get_profile ());
    if (g_str_equal (self->current_profile, NGF_PROFILE_SILENT))
        self->is_silent = TRUE;

    /* Get the vibration enabled status for the current profile. */
    self->is_vibra_enabled = profile_get_value_as_bool (NULL, VIBRATION_ENABLED_KEY) ? TRUE : FALSE;

    /* Get all keys defined in the profiles. We will use this to iterate through
       all the profiles and get the respective values. */

    keys = profile_get_keys ();

    /* Get all the initial entries for each profile defined
       and also for fallbacks. */

    profiles = profile_get_profiles ();

    for (p = profiles; *p; p++)
        _profile_query_values (self, keys, *p);

    _profile_query_values (self, keys, NGF_PROFILE_FALLBACK);

    profile_free_profiles (profiles);
    profiles = NULL;

    profile_free_keys (keys);
    keys = NULL;
}

static void
_track_value_change_cb (const char *profile,
				        const char *key,
				        const char *value,
				        const char *type,
				        void *userdata)
{
    NgfProfile *self = (NgfProfile*) userdata;
    ProfileEntry *entry = NULL;

    if (g_str_equal (key, VIBRATION_ENABLED_KEY) && g_str_equal (profile, self->current_profile))
        self->is_vibra_enabled = profile_parse_bool (value) == 1 ? TRUE : FALSE;

    if ((entry = _profile_entry_find (self, profile)) == NULL) {
        entry = _profile_entry_new (profile);
        self->profiles = g_list_append (self->profiles, entry);
    }

    g_hash_table_replace (entry->values, g_strdup (key), g_strdup (value));
}

static void
_track_profile_change_cb (const char *profile,
                          void *userdata)
{
    NgfProfile *self = (NgfProfile*) userdata;

    g_free (self->current_profile);
    self->current_profile = g_strdup (profile);

    self->is_silent = FALSE;
    if (g_str_equal (self->current_profile, NGF_PROFILE_SILENT))
        self->is_silent = TRUE;
}

NgfProfile*
ngf_profile_create ()
{
    NgfProfile *self = NULL;

    if ((self = g_new0 (NgfProfile, 1)) == NULL)
        return NULL;

    /* Get all the initial values for the profiles */
    _profile_setup (self);

    /* Track the profile value changes in all profiles. */
    profile_track_add_active_cb (_track_value_change_cb, self, NULL);
    profile_track_add_change_cb (_track_value_change_cb, self, NULL);
    profile_track_add_profile_cb (_track_profile_change_cb, self, NULL);

    profile_tracker_init ();

    return self;
}

void
ngf_profile_destroy (NgfProfile *self)
{
    GList *iter = NULL;
    ProfileEntry *entry = NULL;

    if (self == NULL)
        return;

    profile_tracker_quit ();

    if (self->profiles) {
        for (iter = g_list_first (self->profiles); iter; iter = g_list_next (iter)) {
            entry = (ProfileEntry*) iter->data;
            _profile_entry_free (entry);
        }

        g_list_free (self->profiles);
        self->profiles = NULL;
    }

    g_free (self);
}

const char*
ngf_profile_get_current (NgfProfile *self)
{
    return self->current_profile;
}

static const char*
_get_value (NgfProfile *self, const char *profile, const char *key)
{
    ProfileEntry *entry = NULL;
    const char *search_profile = profile;

    if (key == NULL)
        return NULL;

    if (profile == NULL)
        search_profile = (const char*) self->current_profile;

    if ((entry = _profile_entry_find (self, search_profile)) == NULL)
        return NULL;

    return (const char*) g_hash_table_lookup (entry->values, key);
 }

gboolean
ngf_profile_get_string (NgfProfile *self, const char *profile, const char *key, const char **value)
{
    const char *v = NULL;

    if ((v = _get_value (self, profile, key)) == NULL)
        return FALSE;

    *value = v;
    return TRUE;
}

gboolean
ngf_profile_get_integer (NgfProfile *self, const char *profile, const char *key, gint *value)
{
    const char *v = NULL;

    if ((v = _get_value (self, profile, key)) == NULL)
        return FALSE;

    *value = profile_parse_int (v);
    return TRUE;
}

gboolean
ngf_profile_get_boolean (NgfProfile *self, const char *profile, const char *key, gboolean *value)
{
    const char *v = NULL;

    if ((v = _get_value (self, profile, key)) == NULL)
        return FALSE;

    *value = profile_parse_bool (v) ? TRUE : FALSE;
    return TRUE;
}

gboolean
ngf_profile_is_silent (NgfProfile *self)
{
    if (self == NULL)
        return FALSE;

    return self->is_silent;
}

gboolean
ngf_profile_is_vibra_enabled (NgfProfile *self)
{
    if (self == NULL)
        return FALSE;

    return self->is_vibra_enabled;
}

gboolean
ngf_profile_parse_profile_key (const char *key, gchar **out_profile, gchar **out_key)
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

const char*
ngf_profile_get_string_from_key (NgfProfile *self, const char *key)
{
    gchar *profile = NULL, *inkey = NULL;
    const char *value = NULL;

    if (self == NULL || key == NULL)
        return NULL;

    if (!ngf_profile_parse_profile_key (key, &profile, &inkey))
        return NULL;

    ngf_profile_get_string (self, profile, inkey, &value);

    g_free (profile);
    g_free (inkey);
    return value;
}

gint
ngf_profile_get_int_from_key (NgfProfile *self, const char *key)
{
    gchar *profile = NULL, *inkey = NULL;
    gint value = -1;

    if (self == NULL || key == NULL)
        return -1;

    if (!ngf_profile_parse_profile_key (key, &profile, &inkey))
        return -1;

    ngf_profile_get_integer (self, profile, inkey, &value);

    g_free (profile);
    g_free (inkey);
    return value;
}
