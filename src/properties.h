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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <glib.h>

GHashTable* properties_new ();
GHashTable* properties_copy (GHashTable *properties);
void        properties_merge (GHashTable *target, GHashTable *source);
void        properties_merge_allowed (GHashTable *target, GHashTable *source, gchar **allowed_keys);
void        properties_dump (GHashTable *properties);

const char* properties_get_string (GHashTable *source, const char *name);
gint        properties_get_int    (GHashTable *source, const char *name);
gboolean    properties_get_bool   (GHashTable *source, const char *name);

#endif /* PROPERTIES_H */
