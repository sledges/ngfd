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

#include <pulse/pulseaudio.h>
#include <pulse/context.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>

#define APPLICATION_NAME "pulse-context"
#define VOLUME_SCALE_VALUE 50000

#include "log.h"
#include "pulse-context.h"
#include "config.h"

struct _PulseContext
{
    pa_glib_mainloop        *mainloop;
    pa_context              *context;
    PulseContextCallback  callback;
    gpointer                 userdata;
};

/* Function pre-declarations */
static gboolean _pulseaudio_initialize (PulseContext *self);
static void     _pulseaudio_shutdown   (PulseContext *self);
static void     _context_state_cb      (pa_context *context, void *userdata);

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
            if (self->callback)
                self->callback (self, PULSE_CONTEXT_STATE_READY, self->userdata);
            break;

        case PA_CONTEXT_FAILED:
            if (self->callback)
                self->callback (self, PULSE_CONTEXT_STATE_FAILED, self->userdata);
            break;

        case PA_CONTEXT_TERMINATED:
            if (self->callback)
                self->callback (self, PULSE_CONTEXT_STATE_TERMINATED, self->userdata);
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

    if ((self->mainloop = pa_glib_mainloop_new (g_main_context_default ())) == NULL)
        return FALSE;

    if ((api = pa_glib_mainloop_get_api (self->mainloop)) == NULL)
        return FALSE;

    proplist = pa_proplist_new ();
    pa_proplist_sets (proplist, PA_PROP_APPLICATION_NAME, APPLICATION_NAME);
    pa_proplist_sets (proplist, PA_PROP_APPLICATION_ID, APPLICATION_NAME);
    pa_proplist_sets (proplist, PA_PROP_APPLICATION_VERSION, PACKAGE_VERSION);

    self->context = pa_context_new_with_proplist (api, APPLICATION_NAME, proplist);
    if (self->context == NULL) {
        pa_proplist_free (proplist);
	    return FALSE;
    }

	pa_proplist_free (proplist);
    pa_context_set_state_callback (self->context, _context_state_cb, self);

    if (pa_context_connect (self->context, NULL,
        /*PA_CONTEXT_NOFAIL |*/ PA_CONTEXT_NOAUTOSPAWN, NULL) < 0)
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
_pulseaudio_shutdown (PulseContext *self)
{
    g_assert (self != NULL);

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

void
pulse_context_set_volume (PulseContext *self,
                              const char      *role,
                              gint             volume)
{
#ifdef HAVE_STREAMRESTORE
    pa_ext_stream_restore2_info *stream_restore_info[1], info;

    gdouble       v = 0.0;
    pa_operation *o = NULL;
    pa_cvolume    vol;

    if (self->context == NULL || role == NULL || volume < 0)
        return;

    if (pa_context_get_state (self->context) != PA_CONTEXT_READY) {
        while (pa_context_get_state (self->context) != PA_CONTEXT_READY)
            g_main_context_iteration (NULL, 0);
    }

    volume = volume > 100 ? 100 : volume;
    v = (gdouble) volume / 100.0;

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
#endif
}
