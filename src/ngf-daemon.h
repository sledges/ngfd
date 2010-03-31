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

#ifndef NGF_DAEMON_H
#define NGF_DAEMON_H

#include <glib.h>

#include "ngf-event-definition.h"
#include "ngf-event-prototype.h"
#include "ngf-event.h"

#include "ngf-context.h"
#include "ngf-dbus.h"

typedef struct _NgfDaemon NgfDaemon;

struct _NgfDaemon
{
    GMainLoop       *loop;

    /* Event handling */
    GHashTable      *definitions;
    GHashTable      *prototypes;
    GList           *event_list;

    /* D-Bus interface */
    NgfDBus         *dbus;

    /* Context containing the backends. */
    NgfContext      context;
};

NgfDaemon*         ngf_daemon_create ();
void               ngf_daemon_destroy (NgfDaemon *self);
void               ngf_daemon_run (NgfDaemon *self);
guint              ngf_daemon_event_play (NgfDaemon *self, const char *event_name, GHashTable *properties);
void               ngf_daemon_event_stop (NgfDaemon *self, guint id);

void               ngf_daemon_register_definition (NgfDaemon *self, const char *name, NgfEventDefinition *def);
void               ngf_daemon_register_prototype (NgfDaemon *self, const char *name, NgfEventPrototype *proto);
NgfEventPrototype* ngf_daemon_get_prototype (NgfDaemon *self, const char *name);

gboolean           ngf_daemon_settings_load (NgfDaemon *self);

#endif /* NGF_DAEMON_H */
