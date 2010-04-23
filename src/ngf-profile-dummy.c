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

#include "ngf-profile.h"

struct _NgfProfile
{
    int dummy;
};


NgfProfile*
ngf_profile_create ()
{
    NgfProfile *self = NULL;

    if ((self = g_new0 (NgfProfile, 1)) == NULL)
        return NULL;

    return self;
}

void
ngf_profile_destroy (NgfProfile *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

const char*
ngf_profile_get_current (NgfProfile *self)
{
    return NULL;
}

gboolean
ngf_profile_get_string (NgfProfile *self, const char *profile, const char *key, const char **value)
{
    return FALSE;
}

gboolean
ngf_profile_get_integer (NgfProfile *self, const char *profile, const char *key, gint *value)
{
    return FALSE;
}

gboolean
ngf_profile_get_boolean (NgfProfile *self, const char *profile, const char *key, gboolean *value)
{
    return FALSE;
}

gboolean
ngf_profile_is_silent (NgfProfile *self)
{
    return FALSE;
}

gboolean
ngf_profile_is_vibra_enabled (NgfProfile *self)
{
    return FALSE;
}

gboolean
ngf_profile_parse_profile_key (const char *key, gchar **out_profile, gchar **out_key)
{
    return FALSE;
}

const char*
ngf_profile_get_string_from_key (NgfProfile *self, const char *key)
{
    return NULL;
}

gint
ngf_profile_get_int_from_key (NgfProfile *self, const char *key)
{
    return -1;
}
