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

#ifndef NGF_INTERFACE_H
#define NGF_INTERFACE_H

typedef enum  _NgfInterfaceType  NgfInterfaceType;

enum _NgfInterfaceType
{
    NGF_INTERFACE_AUDIO,
    NGF_INTERFACE_VIBRA
};

typedef void (*NgfInterfaceReadyCallback) (NgfInterfaceType type, gboolean success, gpointer userdata);

#endif /* NGF_INTERFACE_H */