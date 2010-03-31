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

#ifndef NGF_PROPERTIES_H
#define NGF_PROPERTIES_H

#include <glib.h>
#include "ngf-value.h"

GHashTable* ngf_properties_new ();
GHashTable* ngf_properties_copy (GHashTable *properties);
void        ngf_properties_merge (GHashTable *target, GHashTable *source);
void        ngf_properties_merge_allowed (GHashTable *target, GHashTable *source, gchar **allowed_keys);
void        ngf_properties_dump (GHashTable *properties);

const char* ngf_properties_get_string (GHashTable *source, const char *name);
gint        ngf_properties_get_int    (GHashTable *source, const char *name);
gboolean    ngf_properties_get_bool   (GHashTable *source, const char *name);

#endif /* NGF_PROPERTIES_H */
