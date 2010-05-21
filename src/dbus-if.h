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

#ifndef DBUS_IF_H
#define DBUS_IF_H

#include <dbus/dbus.h>
#include <glib.h>

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

typedef struct _DBusIf DBusIf;

typedef guint (*DBusIfPlay) (DBusIf *dbus_if, const char *event, GHashTable *properties, gpointer userdata);
typedef void  (*DBusIfStop) (DBusIf *dbus_if, guint id, gpointer userdata);

struct _DBusIf
{
    DBusConnection *connection;
    DBusIfPlay      play_function;
    DBusIfStop      stop_function;
    gpointer        userdata;
};

DBusIf* dbus_if_create      (DBusIfPlay play, DBusIfStop stop, gpointer userdata);
void    dbus_if_destroy     (DBusIf *self);
void    dbus_if_send_status (DBusIf *self, guint id, guint status);

#endif /* DBUSIF_H */
