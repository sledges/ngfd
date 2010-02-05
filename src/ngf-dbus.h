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

#ifndef NGF_DBUS_H
#define NGF_DBUS_H

#include <dbus/dbus.h>
#include <glib.h>

#include "ngf-value.h"

/* D-Bus names and interfaces */
#define NGF_DBUS_PROXY_NAME    "com.nokia.NonGraphicFeedback1"
#define NGF_DBUS_BACKEND_NAME  "com.nokia.NonGraphicFeedback1.Backend"
#define NGF_DBUS_PATH          "/com/nokia/NonGraphicFeedback1"
#define NGF_DBUS_IFACE         "com.nokia.NonGraphicFeedback1"

/* Proxy status method for updating client */
#define NGF_DBUS_STATUS        "Status"

/* Backend supported methods */
#define NGF_DBUS_BACKEND_PLAY  "Play"
#define NGF_DBUS_BACKEND_STOP  "Stop"

typedef struct  _NgfDBus NgfDBus;

typedef guint   (*NgfDBusPlayFunction) (NgfDBus *dbus_if, const char *event, GHashTable *properties, gpointer userdata);
typedef void    (*NgfDBusStopFunction) (NgfDBus *dbus_if, guint id, gpointer userdata);

NgfDBus*        ngf_dbus_create (NgfDBusPlayFunction play, NgfDBusStopFunction stop, gpointer userdata);
void            ngf_dbus_destroy (NgfDBus *self);
void            ngf_dbus_send_status (NgfDBus *self, guint id, guint status);

#endif /* NGF_DBUS_H */
