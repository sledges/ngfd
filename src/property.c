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
