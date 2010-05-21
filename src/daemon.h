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

#ifndef DAEMON_H
#define DAEMON_H

#include <glib.h>
#include "context.h"

Context*        daemon_create ();
void            daemon_destroy (Context *self);
void            daemon_run (Context *self);
guint           daemon_event_play (Context *self, const char *event_name, GHashTable *properties);
void            daemon_event_stop (Context *self, guint id);

void            daemon_register_definition (Context *self, const char *name, EventDefinition *def);
void            daemon_register_prototype (Context *self, const char *name, EventPrototype *proto);
EventPrototype* daemon_get_prototype (Context *self, const char *name);

gboolean        daemon_settings_load (Context *self);

#endif /* DAEMON_H */
