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

#include <ngf/hook.h>
#include <glib.h>
#include <string.h>

typedef struct _NHookSlot
{
    NHook         *hook;
    int            priority;
    NHookCallback  callback;
    void          *userdata;
} NHookSlot;

static void
n_hook_slot_free (NHookSlot *slot)
{
    if (!slot)
        return;

    g_slice_free (NHookSlot, slot);
}

void
n_hook_init (NHook *hook)
{
    memset (hook, 0, sizeof (NHook));
}

static int
n_hook_sort_slot_cb (gconstpointer in_a, gconstpointer in_b)
{
    NHookSlot *a = (NHookSlot*) in_a;
    NHookSlot *b = (NHookSlot*) in_b;

    if (a->priority > b->priority)
        return -1;
    else if (b->priority > a->priority)
        return 1;
    return 0;
}

int
n_hook_connect (NHook *hook, int priority, NHookCallback callback,
                void *userdata)
{
    NHookSlot *slot = NULL;

    if (!hook || !callback)
        return FALSE;

    slot = g_slice_new0 (NHookSlot);
    slot->hook     = hook;
    slot->callback = callback;
    slot->userdata = userdata;
    slot->priority = priority;

    hook->slots = g_list_append (hook->slots, slot);
    hook->slots = g_list_sort (hook->slots, n_hook_sort_slot_cb);

    return TRUE;
}

void
n_hook_disconnect (NHook *hook, NHookCallback callback, void *userdata)
{
    GList *iter = NULL;

    if (!hook || !callback)
        return;

    for (iter = g_list_first (hook->slots); iter; iter = g_list_next (iter)) {
        NHookSlot *slot = (NHookSlot*) iter->data;
        if (slot->callback == callback && slot->userdata == userdata) {
            hook->slots = g_list_remove (hook->slots, slot);
            n_hook_slot_free (slot);
            return;
        }
    }
}

int
n_hook_fire (NHook *hook, void *data)
{
    GList *iter = NULL;

    if (!hook)
        return FALSE;

    for (iter = g_list_first (hook->slots); iter; iter = g_list_next (iter)) {
        NHookSlot *slot = (NHookSlot*) iter->data;
        slot->callback (hook, data, slot->userdata);
    }

    return TRUE;
}

