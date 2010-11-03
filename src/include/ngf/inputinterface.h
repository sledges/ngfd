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

#ifndef N_INPUT_INTERFACE_H
#define N_INPUT_INTERFACE_H

typedef struct _NInputInterface NInputInterface;

#include <ngf/core.h>
#include <ngf/request.h>

typedef struct _NInputInterfaceDecl
{
    const char *name;

    int  (*initialize) (NInputInterface *iface);
    void (*shutdown)   (NInputInterface *iface);
    void (*send_error) (NInputInterface *iface, NRequest *request, const char *err_msg);
    void (*send_reply) (NInputInterface *iface, NRequest *request, int ret_code);
} NInputInterfaceDecl;

NCore* n_input_interface_get_core      (NInputInterface *iface);
int    n_input_interface_play_request  (NInputInterface *iface, NRequest *request);
int    n_input_interface_pause_request (NInputInterface *iface, NRequest *request);
void   n_input_interface_stop_request  (NInputInterface *iface, NRequest *request);

#endif /* N_INPUT_INTERFACE_H */
