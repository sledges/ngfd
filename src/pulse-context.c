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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <pulse/pulseaudio.h>
#include <pulse/context.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>

#define APPLICATION_NAME "pulse-context"
#define VOLUME_SCALE_VALUE 50000
#define PULSE_RECONNECT_DELAY 2

#include "log.h"
#include "pulse-context.h"
#include "config.h"

typedef struct _VolumeAction VolumeAction;

struct _VolumeAction
{
    gchar *role;
    gint   level;
};

struct _PulseContext
{
    pa_glib_mainloop     *mainloop;
    pa_context           *context;
    PulseContextCallback  callback;
    gpointer              userdata;
    GQueue               *actions;
    guint                 volume_id;
    guint                 reconnect_id;
};

/* Function pre-declarations */
static gboolean _pulseaudio_initialize (PulseContext *self);
static void     _pulseaudio_shutdown   (PulseContext *self);
static void     _context_state_cb      (pa_context *context, void *userdata);
static gboolean _process_volume_action (gpointer userdata);

static gboolean
_reconnect_cb (gpointer userdata)
{
    PulseContext *self = (PulseContext*) userdata;
    
    _pulseaudio_initialize (self);
    
    return FALSE;
}

/**
 * Pulseaudio context handling. Pulseaudio will trigger this callback
 * when it's internal state changes.
 *
 * We will mainly trigger AudioCallback's here to let the client
 * know of the current Pulseaudio state and take action if connection is
 * lost.
 *
 * @param context Pulseaudio context.
 * @param userdata PulseContext
 */

static void
_context_state_cb (pa_context *context,
                   void       *userdata)
{
    PulseContext *self = (PulseContext*) userdata;

    switch (pa_context_get_state (context)) {
        case PA_CONTEXT_CONNECTING:
            if (self->callback)
                self->callback (self, PULSE_CONTEXT_STATE_SETUP, self->userdata);
            break;

        case PA_CONTEXT_READY:
            if (self->volume_id > 0) {
                g_source_remove (self->volume_id);
                self->volume_id = 0;
            }

            self->volume_id = g_idle_add (_process_volume_action, self);

            if (self->callback)
                self->callback (self, PULSE_CONTEXT_STATE_READY, self->userdata);
            break;

        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            self->reconnect_id = g_timeout_add_seconds (PULSE_RECONNECT_DELAY, _reconnect_cb, self);
            break;

        default:
            break;
    }
}

/**
 * Initialize the Pulseaudio context and setup the context callback to
 * get notified of the state changes.
 *
 * @param self PulseContext
 * @returns TRUE is successful, FALSE if failed.
 */

static gboolean
_pulseaudio_initialize (PulseContext *self)
{
    pa_proplist     *proplist = NULL;
    pa_mainloop_api *api      = NULL;

    g_assert (self != NULL);
    
    if (self->mainloop) {
        pa_glib_mainloop_free (self->mainloop);
        self->mainloop = NULL;
    }
    
    if ((self->mainloop = pa_glib_mainloop_new (g_main_context_default ())) == NULL)
        return FALSE;

    if ((api = pa_glib_mainloop_get_api (self->mainloop)) == NULL)
        return FALSE;

    proplist = pa_proplist_new ();
    pa_proplist_sets (proplist, PA_PROP_APPLICATION_NAME, APPLICATION_NAME);
    pa_proplist_sets (proplist, PA_PROP_APPLICATION_ID, APPLICATION_NAME);
    pa_proplist_sets (proplist, PA_PROP_APPLICATION_VERSION, PACKAGE_VERSION);
    
    if (self->context) {
        pa_context_unref (self->context);
        self->context = NULL;
    }

    self->context = pa_context_new_with_proplist (api, APPLICATION_NAME, proplist);
    if (self->context == NULL) {
        pa_proplist_free (proplist);
	    return FALSE;
    }

	pa_proplist_free (proplist);
    pa_context_set_state_callback (self->context, _context_state_cb, self);

    if (pa_context_connect (self->context, NULL,
        PA_CONTEXT_NOFAIL | PA_CONTEXT_NOAUTOSPAWN, NULL) < 0)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * Destroy Pulseaudio context and free related resources.
 *
 * @pre PulseContext is valid
 * @param self PulseContext
 */

static void
_free_queue_item (gpointer data, gpointer userdata)
{
    VolumeAction *action = (VolumeAction*) data;

    if (!action)
        return;

    g_free       (action->role);
    g_slice_free (VolumeAction, action);
}

static void
_pulseaudio_shutdown (PulseContext *self)
{
    g_assert (self != NULL);

    if (self->actions) {
        g_queue_foreach (self->actions, _free_queue_item, NULL);
        g_queue_free    (self->actions);
        self->actions = NULL;
    }
    
    if (self->reconnect_id) {
        g_source_remove (self->reconnect_id);
        self->reconnect_id = 0;
    }

    if (self->context) {
        pa_context_set_state_callback (self->context, NULL, NULL);
        pa_context_disconnect (self->context);
        pa_context_unref (self->context);
        self->context = NULL;
    }

    if (self->mainloop) {
        pa_glib_mainloop_free (self->mainloop);
        self->mainloop = NULL;
    }
}

PulseContext*
pulse_context_create ()
{
    PulseContext *self = NULL;

    if ((self = g_new0 (PulseContext, 1)) == NULL)
        goto failed;

    if ((self->actions = g_queue_new ()) == NULL)
        goto failed;

    if (!_pulseaudio_initialize (self))
        goto failed;

    return self;

failed:
    pulse_context_destroy (self);
    return NULL;
}

void
pulse_context_destroy (PulseContext *self)
{
    if (self == NULL)
        return;

    _pulseaudio_shutdown (self);
    g_free (self);
}

void
pulse_context_set_callback (PulseContext         *self,
                                PulseContextCallback callback,
                                gpointer                userdata)
{
    if (self == NULL || callback == NULL)
        return;

    self->callback = callback;
    self->userdata = userdata;
}

pa_context*
pulse_context_get_context (PulseContext *self)
{
    if (self == NULL)
        return NULL;

    return self->context;
}

static void
_set_volume (PulseContext *self, const char *role, gint volume)
{
    if (!self || !role)
        return;

    gdouble v = 0.0;
    volume = volume > 100 ? 100 : volume;
    v = (gdouble) volume / 100.0;

#ifdef HAVE_STREAMRESTORE
    pa_ext_stream_restore2_info *stream_restore_info[1], info;
    pa_operation *o = NULL;
    pa_cvolume    vol;

    pa_cvolume_set (&vol, 1, v * VOLUME_SCALE_VALUE);

    info.name                 = role;
    info.channel_map.channels = 1;
    info.channel_map.map[0]   = PA_CHANNEL_POSITION_MONO;
    info.volume               = vol;
    info.device               = NULL;
    info.mute                 = FALSE;
    info.volume_is_absolute   = TRUE;
    stream_restore_info[0]    = &info;

    o = pa_ext_stream_restore2_write (self->context,
        PA_UPDATE_REPLACE, (const pa_ext_stream_restore2_info *const *) stream_restore_info, 1, TRUE, NULL, NULL);

    if (o) {
        LOG_DEBUG ("%s >> volume for role %s set to %d", __FUNCTION__, role, (guint) (v * PA_VOLUME_NORM));
        pa_operation_unref (o);
    }
    else {
        LOG_DEBUG ("%s >> failed to set volume for role %s (%d)", __FUNCTION__, role, (guint) (v * PA_VOLUME_NORM));
    }
#endif

}

static gboolean
_process_volume_action (gpointer userdata)
{
    PulseContext *self   = (PulseContext*) userdata;
    VolumeAction *action = NULL;

    if (!self->actions)
        return FALSE;

    while ((action = (VolumeAction*) g_queue_pop_head (self->actions)) != NULL) {
        _set_volume  (self, action->role, action->level);
        g_free       (action->role);
        g_slice_free (VolumeAction, action);
    }

    self->volume_id = 0;
    return FALSE;
}

void
pulse_context_set_volume (PulseContext *self,
                          const char   *role,
                          gint          volume)
{
    VolumeAction *action = NULL;

    if (!self || !role)
        return;

    if (pa_context_get_state (self->context) == PA_CONTEXT_READY) {
        _set_volume (self, role, volume);
    }
    else {
        action = g_slice_new0 (VolumeAction);
        action->role  = g_strdup (role);
        action->level = volume;
        g_queue_push_tail (self->actions, action);
    }
}
