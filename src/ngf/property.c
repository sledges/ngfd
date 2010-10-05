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

#include "property.h"

Property*
property_new ()
{
    return g_slice_new0 (Property);
}

Property*
property_copy (Property *p)
{
    Property *new_property = NULL;

    new_property = property_new ();
    new_property->type = p->type;

    switch (p->type) {
        case PROPERTY_TYPE_STRING:
            new_property->value.s = g_strdup (p->value.s);
            break;
        case PROPERTY_TYPE_INT:
            new_property->value.i = p->value.i;
            break;
        case PROPERTY_TYPE_UINT:
            new_property->value.u = p->value.u;
            break;
        case PROPERTY_TYPE_BOOLEAN:
            new_property->value.b = p->value.b;
            break;
        default:
            property_free (new_property);
            return NULL;
    }

    return new_property;
}

static void
_internal_property_free (Property *p)
{
    if (p->type == PROPERTY_TYPE_STRING)
        g_free (p->value.s);

    p->type = 0;
}

void
property_free (Property *p)
{
    if (p == NULL)
        return;

    _internal_property_free (p);
    g_slice_free (Property, p);
}

guint
property_get_type (Property *p)
{
    return p->type;
}

void
property_set_string (Property *p, const char *value)
{
    _internal_property_free (p);
    p->type = PROPERTY_TYPE_STRING;
    p->value.s = g_strdup (value);
}

const char*
property_get_string (Property *p)
{
    return p->value.s;
}

void
property_set_uint (Property *p, guint value)
{
    _internal_property_free (p);
    p->type = PROPERTY_TYPE_UINT;
    p->value.u = value;
}

guint
property_get_uint (Property *p)
{
    return p->value.u;
}

void
property_set_int (Property *p, gint value)
{
    _internal_property_free (p);
    p->type = PROPERTY_TYPE_INT;
    p->value.i = value;
}

gint
property_get_int (Property *p)
{
    return p->value.i;
}

void
property_set_boolean (Property *p, gboolean value)
{
    _internal_property_free (p);
    p->type = PROPERTY_TYPE_BOOLEAN;
    p->value.b = value;
}

gboolean
property_get_boolean (Property *p)
{
    return p->value.b;
}
