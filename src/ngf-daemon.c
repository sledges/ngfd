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

#include "ngf-value.h"
#include "ngf-event.h"
#include "ngf-daemon.h"

static void     _audio_state_cb              (NgfAudio *audio, NgfAudioState state, gpointer userdata);
static void     _event_state_cb              (NgfEvent *event, NgfEventState state, gpointer userdata);
static gboolean _properties_get_boolean      (GHashTable *properties, const char *key);
static guint    _properties_get_policy_id    (GHashTable *properties);
static guint    _properties_get_play_timeout (GHashTable *properties);
static gint     _properties_get_play_mode    (GHashTable *properties);
static gint     _properties_get_resources    (GHashTable *properties);
static guint    _handle_play_cb              (NgfDBus *dbus, const char *event, GHashTable *properties, gpointer userdata);
static void     _handle_stop_cb              (NgfDBus *dbus, guint id, gpointer userdata);

NgfDaemon*
ngf_daemon_create ()
{
    NgfDaemon *self = NULL;

    if ((self = g_new0 (NgfDaemon, 1)) == NULL)
        return NULL;

    if ((self->loop = g_main_loop_new (NULL, 0)) == NULL)
        return NULL;

    if ((self->event_manager = ngf_event_manager_create ()) == NULL)
        return NULL;

    if ((self->context.profile = ngf_profile_create ()) == NULL)
        return NULL;

    if ((self->context.audio = ngf_audio_create ()) == NULL)
        return NULL;

    if ((self->context.vibrator = ngf_vibrator_create ()) == NULL)
        return NULL;

    if ((self->context.tonegen = ngf_tonegen_create ()) == NULL)
        return NULL;

    if ((self->context.led = ngf_led_create ()) == NULL)
        return NULL;

    if (!ngf_daemon_settings_load (self))
        return NULL;

    if ((self->dbus = ngf_dbus_create (_handle_play_cb, _handle_stop_cb, self)) == NULL)
        return NULL;

    return self;
}

void
ngf_daemon_destroy (NgfDaemon *self)
{
    if (self->dbus) {
        ngf_dbus_destroy (self->dbus);
        self->dbus = NULL;
    }

    if (self->context.led) {
        ngf_led_destroy (self->context.led);
        self->context.led = NULL;
    }

    if (self->context.tonegen) {
        ngf_tonegen_destroy (self->context.tonegen);
        self->context.tonegen = NULL;
    }

    if (self->context.vibrator) {
        ngf_vibrator_destroy (self->context.vibrator);
        self->context.vibrator = NULL;
    }

    if (self->context.audio) {
        ngf_audio_destroy (self->context.audio);
        self->context.audio = NULL;
    }

    if (self->context.profile) {
        ngf_profile_destroy (self->context.profile);
        self->context.profile = NULL;
    }

    if (self->event_manager) {
        ngf_event_manager_destroy (self->event_manager);
        self->event_manager = NULL;
    }

    if (self->loop) {
        g_main_loop_unref (self->loop);
        self->loop = NULL;
    }

    g_free (self);
}

void
ngf_daemon_run (NgfDaemon *self)
{
    g_main_loop_run (self->loop);
}

static gboolean
_properties_get_boolean (GHashTable *properties, const char *key)
{
    NgfValue *value = NULL;

    if ((value = g_hash_table_lookup (properties, key)) == NULL)
        return FALSE;

    if (ngf_value_get_type (value) == NGF_VALUE_BOOLEAN)
        return ngf_value_get_boolean (value);

    return FALSE;
}

static guint
_properties_get_policy_id (GHashTable *properties)
{
    NgfValue *value = NULL;

    if ((value = g_hash_table_lookup (properties, "policy.id")) == NULL)
        return 0;

    if (ngf_value_get_type (value) == NGF_VALUE_UINT)
        return ngf_value_get_uint (value);

    return 0;
}

static guint
_properties_get_play_timeout (GHashTable *properties)
{
    NgfValue *value = NULL;

    if ((value = g_hash_table_lookup (properties, "play.timeout")) == NULL)
        return 0;

    if (ngf_value_get_type (value) == NGF_VALUE_UINT)
        return ngf_value_get_uint (value);

    return 0;
}

static gint
_properties_get_play_mode (GHashTable *properties)
{
    NgfValue *value = NULL;
    const char *str = NULL;

    if ((value = g_hash_table_lookup (properties, "play.mode")) == NULL)
        return 0;

    if (ngf_value_get_type (value) == NGF_VALUE_STRING) {
        str = ngf_value_get_string (value);

        if (g_str_equal (str, "short"))
            return NGF_PLAY_MODE_SHORT;
        else if (g_str_equal (str, "long"))
            return NGF_PLAY_MODE_LONG;
    }

    return 0;
}

static gint
_properties_get_resources (GHashTable *properties)
{
    NgfValue *value = NULL;
    gint resources = 0;

    if (_properties_get_boolean (properties, "media.audio"))
        resources |= NGF_RESOURCE_AUDIO;

    if (_properties_get_boolean (properties, "media.vibra"))
        resources |= NGF_RESOURCE_VIBRATION;

    if (_properties_get_boolean (properties, "media.leds"))
        resources |= NGF_RESOURCE_LED;

    if (_properties_get_boolean (properties, "media.backlight"))
        resources |= NGF_RESOURCE_BACKLIGHT;

    return resources;
}

static void
_event_state_cb (NgfEvent *event, NgfEventState state, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    gboolean remove_event = FALSE;

    switch (state) {
        case NGF_EVENT_STARTED:
            g_print ("EVENT STARTED (id=%d)\n", event->policy_id);
            break;

        case NGF_EVENT_FAILED:
            g_print ("EVENT FAILED (id=%d)\n", event->policy_id);
            ngf_dbus_send_status (self->dbus, event->policy_id, 0);
            remove_event = TRUE;
            break;

        case NGF_EVENT_COMPLETED:
            g_print ("EVENT COMPLETED (id=%d)\n", event->policy_id);
            ngf_dbus_send_status (self->dbus, event->policy_id, 0);
            remove_event = TRUE;
            break;

        default:
            break;
    }

    if (remove_event) {
        g_print ("EVENT REMOVED (id=%d)\n", event->policy_id);
        self->event_list = g_list_remove (self->event_list, event);
        ngf_event_stop (event);
        ngf_event_free (event);
    }
}

guint
ngf_daemon_event_play (NgfDaemon *self, const char *event_name, GHashTable *properties)
{
    NgfEventDefinition *def = NULL;
    NgfEventPrototype *proto = NULL;
    NgfEvent *event = NULL;

    guint policy_id = 0, play_timeout = 0;
    gint resources = 0, play_mode = 0;
    const char *proto_name = NULL;

    /* First, look for the event definition that defines the prototypes for long and
       short events. If not found, then it is an unrecognized event. */

    if ((def = ngf_event_manager_get_definition (self->event_manager, event_name)) == NULL)
        goto failed;

    /* Then, get the play mode from the passed properties. The play mode is either "long"
       or "short" and we have a corresponding prototypes defined in the event definition.

       If no play mode then this is an invalid event. */

    if ((play_mode = _properties_get_play_mode (properties)) == 0)
        goto failed;

    /* Lookup the prototype based on the play mode. If not found, we have the definition,
       but no actions specified for the play mode. */

    proto_name = (play_mode == NGF_PLAY_MODE_LONG) ? def->long_proto : def->short_proto;

    if ((proto = ngf_event_manager_get_prototype (self->event_manager, proto_name)) == 0)
        goto failed;

    /* Get the policy identifier, allowed resources and play mode and timeout
       for our event. */

    policy_id    = _properties_get_policy_id (properties);
    play_timeout = _properties_get_play_timeout (properties);
    resources    = _properties_get_resources (properties);

    if (policy_id == 0 || resources == 0)
        goto failed;

    g_print ("EVENT (event_name=%s, proto_name=%s, policy_id=%d, play_timeout=%d, resources=0x%X, play_mode=%d (%s))\n",
		    event_name, proto_name, policy_id, play_timeout, resources, play_mode, play_mode == NGF_PLAY_MODE_LONG ? "LONG" : "SHORT");

    /* Create a new event based on the prototype and feed the properties
       to it. */

    if ((event = ngf_event_new (&self->context, proto)) == NULL)
        goto failed;

    event->policy_id    = policy_id;
    event->resources    = resources;
    event->play_mode    = play_mode;
    event->play_timeout = play_timeout;

    /* Setup the event callback to monitor events progress. */

    ngf_event_set_callback (event, _event_state_cb, self);

    /* Start the event immediately. */

    self->event_list = g_list_append (self->event_list, event);
    if (!ngf_event_start (event, properties)) {
        self->event_list = g_list_remove (self->event_list, event);
        ngf_event_free (event);
        return 0;
    }

    return 1;

failed:
    g_hash_table_destroy (properties);
    return 0;
}

void
ngf_daemon_event_stop (NgfDaemon *self, guint id)
{
    NgfEvent *event = NULL;
    GList *iter = NULL;

    if (self->event_list == NULL)
        return;

    for (iter = g_list_first (self->event_list); iter; iter = g_list_next (self->event_list)) {
        event = (NgfEvent*) iter->data;
        if (event->policy_id == id) {
            g_print ("EVENT STOP (id=%d)\n", id);
            self->event_list = g_list_remove (self->event_list, event);
            ngf_event_stop (event);
            ngf_dbus_send_status (self->dbus, event->policy_id, 0);
            ngf_event_free (event);
            break;
        }
    }
}

static guint
_handle_play_cb (NgfDBus *dbus, const char *event, GHashTable *properties, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    return ngf_daemon_event_play (self, event, properties);
}

static void
_handle_stop_cb (NgfDBus *dbus, guint id, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    ngf_daemon_event_stop (self, id);
}
