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

#ifndef NGF_EVENT_MANAGER_H
#define NGF_EVENT_MANAGER_H

#include <glib.h>

#include "ngf-event-prototype.h"
#include "ngf-event.h"

typedef struct _NgfEventManager
{
    GHashTable  *prototypes;
} NgfEventManager;

NgfEventManager*    ngf_event_manager_create ();
void                ngf_event_manager_destroy (NgfEventManager *self);

void                ngf_event_manager_register_prototype (NgfEventManager *self, const char *name, NgfEventPrototype *proto);
NgfEventPrototype*  ngf_event_manager_get_prototype (NgfEventManager *self, const char *name);

#endif /* NGF_EVENT_MANAGER_H */
