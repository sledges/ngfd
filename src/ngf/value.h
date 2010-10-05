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

#ifndef N_VALUE_H
#define N_VALUE_H

#include <glib.h>

typedef enum
{
    N_VALUE_TYPE_STRING = 1,
    N_VALUE_TYPE_INT,
    N_VALUE_TYPE_UINT,
    N_VALUE_TYPE_BOOL,
    N_VALUE_TYPE_POINTER
} NValueType;

typedef struct _NValue NValue;

NValue*      n_value_new         ();
void         n_value_free        (NValue *value);
void         n_value_init        (NValue *value);
void         n_value_clean       (NValue *value);
NValue*      n_value_copy        (NValue *value);
int          n_value_type        (NValue *value);

void         n_value_set_string  (NValue *value, const char *in_value);
const gchar* n_value_get_string  (NValue *value);
gchar*       n_value_dup_string  (NValue *value);
void         n_value_set_int     (NValue *value, gint in_value);
gint         n_value_get_int     (NValue *value);
void         n_value_set_uint    (NValue *value, guint in_value);
guint        n_value_get_uint    (NValue *value);
void         n_value_set_bool    (NValue *value, gboolean in_value);
gboolean     n_value_get_bool    (NValue *value);
void         n_value_set_pointer (NValue *value, gpointer in_value);
gpointer     n_value_get_pointer (NValue *value);

gchar*       n_value_to_string   (NValue *value);

#endif /* N_VALUE_H */
