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

#ifndef N_REQUEST_H
#define N_REQUEST_H

typedef struct _NRequest NRequest;

#include <ngf/proplist.h>

NRequest*        n_request_new            ();
NRequest*        n_request_new_with_event (const char *event);
void             n_request_free           (NRequest *request);
unsigned int     n_request_get_id         (NRequest *request);
const char*      n_request_get_name       (NRequest *request);
void             n_request_set_properties (NRequest *request, NProplist *properties);
const NProplist* n_request_get_properties (NRequest *request);
void             n_request_store_data     (NRequest *request, const char *key, void *data);
void*            n_request_get_data       (NRequest *request, const char *key);
int              n_request_is_paused      (NRequest *request);

NRequest*        n_request_new_with_event_and_properties (const char *event, const NProplist *properties);

#endif /* N_REQUEST_H */
