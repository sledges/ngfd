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
