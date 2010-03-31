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

#include "ngf-value.h"

NgfValue*
ngf_value_new ()
{
    return g_slice_new0 (NgfValue);
}

NgfValue*
ngf_value_copy (NgfValue *value)
{
    NgfValue *new_value = NULL;

    new_value = ngf_value_new ();
    new_value->value_type = value->value_type;
    switch (value->value_type) {
        case NGF_VALUE_STRING:
            new_value->values.str_value = g_strdup (value->values.str_value);
            break;
        case NGF_VALUE_INT:
            new_value->values.int_value = value->values.int_value;
            break;
        case NGF_VALUE_UINT:
            new_value->values.uint_value = value->values.uint_value;
            break;
        case NGF_VALUE_BOOLEAN:
            new_value->values.boolean_value = value->values.boolean_value;
            break;
        default:
            ngf_value_free (new_value);
            return NULL;
    }

    return new_value;
}

static void
_internal_value_free (NgfValue *value)
{
    if (value && value->value_type == NGF_VALUE_STRING)
        g_free (value->values.str_value);
    value->value_type = 0;
}

void
ngf_value_free (NgfValue *value)
{
    if (value == NULL)
        return;

    _internal_value_free (value);
    g_slice_free (NgfValue, value);
}

NgfValueType
ngf_value_get_type (NgfValue *value)
{
    return value->value_type;
}

void
ngf_value_set_string (NgfValue *value, const char *str_value)
{
    _internal_value_free (value);
    value->value_type = NGF_VALUE_STRING;
    value->values.str_value = g_strdup (str_value);
}

const char*
ngf_value_get_string (NgfValue *value)
{
    return value->values.str_value;
}

void
ngf_value_set_uint (NgfValue *value, guint uint_value)
{
    _internal_value_free (value);
    value->value_type = NGF_VALUE_UINT;
    value->values.uint_value = uint_value;
}

guint
ngf_value_get_uint (NgfValue *value)
{
    return value->values.uint_value;
}

void
ngf_value_set_int (NgfValue *value, gint int_value)
{
    _internal_value_free (value);
    value->value_type = NGF_VALUE_INT;
    value->values.int_value = int_value;
}

gint
ngf_value_get_int (NgfValue *value)
{
    return value->values.int_value;
}

void
ngf_value_set_boolean (NgfValue *value, gboolean boolean_value)
{
    _internal_value_free (value);
    value->value_type = NGF_VALUE_BOOLEAN;
    value->values.boolean_value = boolean_value;
}

gboolean
ngf_value_get_boolean (NgfValue *value)
{
    return value->values.boolean_value;
}
