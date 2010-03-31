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

#ifndef NGF_VALUE_H
#define NGF_VALUE_H

#include <glib.h>

typedef enum _NgfValueType
{
    NGF_VALUE_NONE,
    NGF_VALUE_STRING,
    NGF_VALUE_UINT,
    NGF_VALUE_INT,
    NGF_VALUE_BOOLEAN
} NgfValueType;

typedef struct _NgfValue
{
    NgfValueType    value_type;
    union {
        gchar       *str_value;
        guint       uint_value;
        gint        int_value;
        gboolean    boolean_value;
    } values;
} NgfValue;

NgfValue*       ngf_value_new ();
NgfValue*       ngf_value_copy (NgfValue *value);
void            ngf_value_free (NgfValue *value);

NgfValueType    ngf_value_get_type (NgfValue *value);

void            ngf_value_set_string (NgfValue *value, const char *str_value);
const char*     ngf_value_get_string (NgfValue *value);

void            ngf_value_set_uint (NgfValue *value, guint uint_value);
guint           ngf_value_get_uint (NgfValue *value);

void            ngf_value_set_int (NgfValue *value, gint int_value);
gint            ngf_value_get_int (NgfValue *value);

void            ngf_value_set_boolean (NgfValue *value, gboolean boolean_value);
gboolean        ngf_value_get_boolean (NgfValue *value);

#endif /* NGF_VALUE_H */
