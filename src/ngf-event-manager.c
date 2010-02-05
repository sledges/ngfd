/*
 * ngfd - Non-graphical feedback daemon
 * This file is part of ngfd.
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

#include "ngf-event-manager.h"

NgfEventManager*
ngf_event_manager_create ()
{
    NgfEventManager *self = NULL;

    if ((self = g_new0 (NgfEventManager, 1)) == NULL)
        goto failed;

    self->prototypes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) ngf_event_prototype_free);
    if (self->prototypes == NULL)
        goto failed;

    return self;

failed:
    ngf_event_manager_destroy (self);
    return NULL;
}

void
ngf_event_manager_destroy (NgfEventManager *self)
{
    if (self == NULL)
        return;

    if (self->prototypes) {
        g_hash_table_destroy (self->prototypes);
        self->prototypes = NULL;
    }

    g_free (self);
}

void
ngf_event_manager_register_prototype (NgfEventManager *self, const char *name, NgfEventPrototype *proto)
{
    if (self == NULL || name == NULL || proto == NULL)
        return;

    g_hash_table_replace (self->prototypes, g_strdup (name), proto);
}

NgfEventPrototype*
ngf_event_manager_get_prototype (NgfEventManager *self, const char *name)
{
    if (self == NULL || name == NULL)
        return NULL;

    return (NgfEventPrototype*) g_hash_table_lookup (self->prototypes, name);
}
