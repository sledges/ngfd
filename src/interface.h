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

#ifndef INTERFACE_H
#define INTERFACE_H

typedef enum  _InterfaceType  InterfaceType;

enum _InterfaceType
{
    INTERFACE_AUDIO,
    INTERFACE_VIBRA
};

typedef void (*InterfaceReadyCallback) (InterfaceType type, gboolean success, gpointer userdata);

#endif /* INTERFACE_H */
