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

#ifndef N_SINK_INTERFACE_H
#define N_SINK_INTERFACE_H

typedef struct _NSinkInterface NSinkInterface;

#include <ngf/request.h>

typedef struct _NSinkInterfaceDecl
{
    const char *name;

    int  (*initialize) (NSinkInterface *iface);
    void (*shutdown)   (NSinkInterface *iface);
    int  (*can_handle) (NSinkInterface *iface, NRequest *request);
    int  (*prepare)    (NSinkInterface *iface, NRequest *request);
    int  (*play)       (NSinkInterface *iface, NRequest *request);
    int  (*pause)      (NSinkInterface *iface, NRequest *request);
    void (*stop)       (NSinkInterface *iface, NRequest *request);
} NSinkInterfaceDecl;

void n_sink_interface_synchronize (NSinkInterface *iface, NRequest *request);
void n_sink_interface_complete    (NSinkInterface *iface, NRequest *request);
void n_sink_interface_fail        (NSinkInterface *iface, NRequest *request);

#endif /* N_SINK_INTERFACE_H */
