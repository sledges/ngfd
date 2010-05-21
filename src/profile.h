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

#ifndef PROFILE_H
#define PROFILE_H

#include <glib.h>

#define PROFILE_GENERAL     "general"
#define PROFILE_SILENT      "silent"
#define PROFILE_MEETING     "meeting"
#define PROFILE_LOUD        "loud"
#define PROFILE_FALLBACK    "fallback"

typedef struct _Profile Profile;

Profile*    profile_create ();
void        profile_destroy (Profile *self);

const char* profile_get_current (Profile *self);

gboolean    profile_get_string (Profile *self, const char *profile, const char *key, const char **value);
gboolean    profile_get_integer (Profile *self, const char *profile, const char *key, gint *value);
gboolean    profile_get_boolean (Profile *self, const char *profile, const char *key, gboolean *value);

gboolean    profile_is_silent (Profile *self);
gboolean    profile_is_vibra_enabled (Profile *self);

gboolean    profile_parse_profile_key (const char *key, gchar **out_profile, gchar **out_key);
const char* profile_get_string_from_key (Profile *self, const char *key);
gint        profile_get_int_from_key (Profile *self, const char *key);

#endif /* PROFILE_H */
