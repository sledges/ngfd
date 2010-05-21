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

#include "event-definition.h"
#include "event-prototype.h"
#include "event.h"

#include "context.h"
#include "dbus-if.h"

typedef struct _Daemon Daemon;

struct _Daemon
{
    GMainLoop       *loop;

    /* Event handling */
    GHashTable      *definitions;
    GHashTable      *prototypes;
    GList           *event_list;

    /* D-Bus interface */
    DBusIf          *dbus;

    /* Context containing the backends. */
    Context          context;
};

Daemon*         daemon_create ();
void            daemon_destroy (Daemon *self);
void            daemon_run (Daemon *self);
guint           daemon_event_play (Daemon *self, const char *event_name, GHashTable *properties);
void            daemon_event_stop (Daemon *self, guint id);

void            daemon_register_definition (Daemon *self, const char *name, EventDefinition *def);
void            daemon_register_prototype (Daemon *self, const char *name, EventPrototype *proto);
EventPrototype* daemon_get_prototype (Daemon *self, const char *name);

gboolean        daemon_settings_load (Daemon *self);

#endif /* DAEMON_H */
