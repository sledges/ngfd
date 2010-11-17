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

#include <ngf/log.h>
#include <ngf/proplist.h>

#include "context-internal.h"

#define LOG_CAT "context: "

typedef struct _NContextSubscriber
{
    gchar    *key;
    gpointer  userdata;
    NContextValueChangeFunc callback;
} NContextSubscriber;

struct _NContext
{
    NProplist  *values;
    GList      *subscribers;
};

static void
n_context_broadcast_change (NContext *context, const char *key,
                            const NValue *old_value, const NValue *new_value)
{
    NContextSubscriber *subscriber = NULL;
    GList              *iter       = NULL;
    gchar              *old_str    = NULL;
    gchar              *new_str    = NULL;

    old_str = n_value_to_string ((NValue*) old_value);
    new_str = n_value_to_string ((NValue*) new_value);

    N_DEBUG (LOG_CAT "broadcasting value change for '%s': %s -> %s", key,
        old_str, new_str);

    g_free (new_str);
    g_free (old_str);

    for (iter = g_list_first (context->subscribers); iter; iter = g_list_next (iter)) {
        subscriber = (NContextSubscriber*) iter->data;

        if (!subscriber->key || (subscriber->key && g_str_equal (subscriber->key, key))) {
            subscriber->callback (context, key, old_value, new_value, subscriber->userdata);
        }
    }
 }

void
n_context_set_value (NContext *context, const char *key,
                     NValue *value)
{
    NValue *old_value = NULL;

    if (!context || !key)
        return;

    old_value = n_value_copy (n_proplist_get (context->values, key));
    n_proplist_set (context->values, key, value);
    n_context_broadcast_change (context, key, old_value, value);
    n_value_free (old_value);
}

const NValue*
n_context_get_value (NContext *context, const char *key)
{
    if (!context || !key)
        return NULL;

    return (const NValue*) n_proplist_get (context->values, key);
}

int
n_context_subscribe_value_change (NContext *context, const char *key,
                                  NContextValueChangeFunc callback,
                                  void *userdata)
{
    NContextSubscriber *subscriber = NULL;

    if (!context || !callback)
        return FALSE;

    subscriber = g_slice_new0 (NContextSubscriber);
    subscriber->key      = g_strdup (key);
    subscriber->callback = callback;
    subscriber->userdata = userdata;

    context->subscribers = g_list_append (context->subscribers, subscriber);

    N_DEBUG (LOG_CAT "subscriber added for key '%s'", key ? key : "<all keys>");

    return TRUE;
}

void
n_context_unsubscribe_value_change (NContext *context, const char *key,
                                    NContextValueChangeFunc callback)
{
    NContextSubscriber *subscriber = NULL;
    GList *iter = NULL;

    if (!context || !key || !callback)
        return;

    for (iter = g_list_first (context->subscribers); iter; iter = g_list_next (iter)) {
        subscriber = (NContextSubscriber*) iter->data;

        if (g_str_equal (subscriber->key, key) && subscriber->callback == callback) {
            context->subscribers = g_list_remove (context->subscribers, subscriber);
            g_free (subscriber->key);
            g_slice_free (NContextSubscriber, subscriber);
            break;
        }
    }
}

NContext*
n_context_new ()
{
    NContext *context = NULL;

    context = g_new0 (NContext, 1);
    context->values = n_proplist_new ();
    return context;
}

void
n_context_free (NContext *context)
{
    NContextSubscriber *subscriber = NULL;
    GList *iter = NULL;

    for (iter = context->subscribers; iter; iter = g_list_next (iter)) {
        subscriber = (NContextSubscriber*) iter->data;
        g_free (subscriber->key);
        g_slice_free (NContextSubscriber, subscriber);
    }

    g_list_free (context->subscribers);
    context->subscribers = NULL;

    n_proplist_free (context->values);
    g_free (context);
}
