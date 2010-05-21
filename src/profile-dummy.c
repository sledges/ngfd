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

#include "profile.h"

struct _Profile
{
    int dummy;
};


Profile*
profile_create ()
{
    Profile *self = NULL;

    if ((self = g_new0 (Profile, 1)) == NULL)
        return NULL;

    return self;
}

void
profile_destroy (Profile *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

const char*
profile_get_current (Profile *self)
{
    return NULL;
}

gboolean
profile_get_string (Profile *self, const char *profile, const char *key, const char **value)
{
    return FALSE;
}

gboolean
profile_get_integer (Profile *self, const char *profile, const char *key, gint *value)
{
    return FALSE;
}

gboolean
profile_get_boolean (Profile *self, const char *profile, const char *key, gboolean *value)
{
    return FALSE;
}

gboolean
profile_is_silent (Profile *self)
{
    return FALSE;
}

gboolean
profile_is_vibra_enabled (Profile *self)
{
    return FALSE;
}

gboolean
profile_parse_profile_key (const char *key, gchar **out_profile, gchar **out_key)
{
    return FALSE;
}

const char*
profile_get_string_from_key (Profile *self, const char *key)
{
    return NULL;
}

gint
profile_get_int_from_key (Profile *self, const char *key)
{
    return -1;
}
