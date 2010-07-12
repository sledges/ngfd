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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PROPERTY_H
#define PROPERTY_H

#include <glib.h>

enum
{
    PROPERTY_TYPE_NONE,
    PROPERTY_TYPE_STRING,
    PROPERTY_TYPE_UINT,
    PROPERTY_TYPE_INT,
    PROPERTY_TYPE_BOOLEAN
};

typedef struct _Property Property;

struct _Property
{
    guint           type;
    union {
        gchar      *s;
        guint       u;
        gint        i;
        gboolean    b;
    } value;
};

Property*   property_new         ();
Property*   property_copy        (Property *p);
void        property_free        (Property *p);

guint       property_get_type    (Property *p);

void        property_set_string  (Property *p, const char *value);
const char* property_get_string  (Property *p);

void        property_set_uint    (Property *p, guint value);
guint       property_get_uint    (Property *p);

void        property_set_int     (Property *p, gint value);
gint        property_get_int     (Property *p);

void        property_set_boolean (Property *p, gboolean value);
gboolean    property_get_boolean (Property *p);

#endif /* PROPERTY_H */
