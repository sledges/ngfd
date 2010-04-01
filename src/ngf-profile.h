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

#ifndef NGF_PROFILE_H
#define NGF_PROFILE_H

#include <glib.h>

#define NGF_PROFILE_GENERAL     "general"
#define NGF_PROFILE_SILENT      "silent"
#define NGF_PROFILE_MEETING     "meeting"
#define NGF_PROFILE_LOUD        "loud"
#define NGF_PROFILE_FALLBACK    "fallback"

typedef struct _NgfProfile NgfProfile;

NgfProfile* ngf_profile_create ();
void        ngf_profile_destroy (NgfProfile *self);

const char* ngf_profile_get_current (NgfProfile *self);

gboolean    ngf_profile_get_string (NgfProfile *self, const char *profile, const char *key, const char **value);
gboolean    ngf_profile_get_integer (NgfProfile *self, const char *profile, const char *key, gint *value);
gboolean    ngf_profile_get_boolean (NgfProfile *self, const char *profile, const char *key, gboolean *value);

gboolean    ngf_profile_is_silent (NgfProfile *self);
gboolean    ngf_profile_is_vibra_enabled (NgfProfile *self);

#endif /* NGF_PROFILE_H */
