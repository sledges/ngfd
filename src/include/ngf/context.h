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

#ifndef N_CONTEXT_H
#define N_CONTEXT_H

typedef struct _NContext NContext;

#include <ngf/value.h>

typedef void (*NContextValueChangeFunc) (NContext *context,
                                         const char *key,
                                         const NValue *old_value,
                                         const NValue *new_value,
                                         void *userdata);

void          n_context_set_value                (NContext *context, const char *key,
                                                  NValue *value);
const NValue* n_context_get_value                (NContext *context, const char *key);
int           n_context_subscribe_value_change   (NContext *context, const char *key,
                                                  NContextValueChangeFunc callback,
                                                  void *userdata);
void          n_context_unsubscribe_value_change (NContext *context, const char *key,
                                                  NContextValueChangeFunc callback);

#endif /* N_CONTEXT_H */
