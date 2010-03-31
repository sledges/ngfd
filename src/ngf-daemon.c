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

#include "ngf-log.h"
#include "ngf-value.h"
#include "ngf-daemon.h"

static gboolean     _event_manager_create         (NgfDaemon *self);
static void         _event_manager_destroy        (NgfDaemon *self);
NgfEventDefinition* _event_manager_get_definition (NgfDaemon *self, const char *name);
static void         _event_state_cb               (NgfEvent *event, NgfEventState state, gpointer userdata);
static gboolean     _properties_get_boolean       (GHashTable *properties, const char *key);
static guint        _properties_get_policy_id     (GHashTable *properties);
static guint        _properties_get_play_timeout  (GHashTable *properties);
static gint         _properties_get_play_mode     (GHashTable *properties);
static gint         _properties_get_resources     (GHashTable *properties);
static guint        _handle_play_cb               (NgfDBus *dbus, const char *event, GHashTable *properties, gpointer userdata);
static void         _handle_stop_cb               (NgfDBus *dbus, guint id, gpointer userdata);

NgfDaemon*
ngf_daemon_create ()
{
    NgfDaemon *self = NULL;

    if ((self = g_new0 (NgfDaemon, 1)) == NULL) {
        LOG_ERROR ("Failed to allocate memory for NgfDaemon instance!");
        return NULL;
    }

    if ((self->loop = g_main_loop_new (NULL, 0)) == NULL) {
        LOG_ERROR ("Failed to create the GLib mainloop!");
        return NULL;
    }

    if ((self->dbus = ngf_dbus_create (_handle_play_cb, _handle_stop_cb, self)) == NULL) {
        LOG_ERROR ("Failed to create D-Bus interface!");
        return NULL;
    }

    if ((self->context.profile = ngf_profile_create ()) == NULL) {
        LOG_ERROR ("Failed to create profile tracking!");
        return NULL;
    }

    if ((self->context.tone_mapper = ngf_tone_mapper_create ()) == NULL) {
        LOG_WARNING ("Failed to create tone mapper!");
    }

    if ((self->context.audio = ngf_audio_create ()) == NULL) {
        LOG_ERROR ("Failed to create Pulseaudio backend!");
        return NULL;
    }

    if ((self->context.vibrator = ngf_vibrator_create ()) == NULL) {
        LOG_ERROR ("Failed to create Immersion backend!");
        return NULL;
    }

    if ((self->context.tonegen = ngf_tonegen_create ()) == NULL) {
        LOG_ERROR ("Failed to create tone generator backend!");
        return NULL;
    }

    if ((self->context.led = ngf_led_create ()) == NULL) {
        LOG_ERROR ("Failed to create LED backend!");
        return NULL;
    }

    if ((self->context.backlight = ngf_backlight_create ()) == NULL) {
        LOG_ERROR ("Failed to create backlight backend!");
        return NULL;
    }

    if (!_event_manager_create (self)) {
        LOG_ERROR ("Failed to create event manager!");
        return NULL;
    }

    if (!ngf_daemon_settings_load (self)) {
        LOG_ERROR ("Failed to load settings!");
        return NULL;
    }

    return self;
}

void
ngf_daemon_destroy (NgfDaemon *self)
{
    if (self->dbus) {
        ngf_dbus_destroy (self->dbus);
        self->dbus = NULL;
    }

    if (self->context.backlight) {
        ngf_backlight_destroy (self->context.backlight);
	    self->context.backlight = NULL;
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

    if (self->context.tone_mapper) {
        ngf_tone_mapper_destroy (self->context.tone_mapper);
        self->context.tone_mapper = NULL;
    }

    if (self->context.profile) {
        ngf_profile_destroy (self->context.profile);
        self->context.profile = NULL;
    }

    _event_manager_destroy (self);

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
_event_manager_create (NgfDaemon *self)
{
    self->definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) ngf_event_definition_free);
    if (self->definitions == NULL)
        return FALSE;

    self->prototypes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) ngf_event_prototype_free);
    if (self->prototypes == NULL)
        return FALSE;

    return TRUE;
}

static void
_event_manager_destroy (NgfDaemon *self)
{
    if (self->prototypes) {
        g_hash_table_destroy (self->prototypes);
        self->prototypes = NULL;
    }

    if (self->definitions) {
        g_hash_table_destroy (self->definitions);
        self->definitions = NULL;
    }
}

void
ngf_daemon_register_definition (NgfDaemon *self, const char *name, NgfEventDefinition *def)
{
    if (self == NULL || name == NULL || def == NULL)
        return;

    g_hash_table_replace (self->definitions, g_strdup (name), def);
}

NgfEventDefinition*
_event_manager_get_definition (NgfDaemon *self, const char *name)
{
    if (self == NULL || name == NULL)
        return NULL;

    return (NgfEventDefinition*) g_hash_table_lookup (self->definitions, name);
}

void
ngf_daemon_register_prototype (NgfDaemon *self, const char *name, NgfEventPrototype *proto)
{
    if (self == NULL || name == NULL || proto == NULL)
        return;

    g_hash_table_replace (self->prototypes, g_strdup (name), proto);
}

NgfEventPrototype*
ngf_daemon_get_prototype (NgfDaemon *self, const char *name)
{
    if (self == NULL || name == NULL)
        return NULL;

    return (NgfEventPrototype*) g_hash_table_lookup (self->prototypes, name);
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
            LOG_DEBUG ("EVENT STARTED (id=%d)", event->policy_id);
            break;

        case NGF_EVENT_FAILED:
            LOG_DEBUG ("EVENT FAILED (id=%d)", event->policy_id);
            ngf_dbus_send_status (self->dbus, event->policy_id, 0);
            remove_event = TRUE;
            break;

        case NGF_EVENT_COMPLETED:
            LOG_DEBUG ("EVENT COMPLETED (id=%d)", event->policy_id);
            ngf_dbus_send_status (self->dbus, event->policy_id, 0);
            remove_event = TRUE;
            break;

        default:
            break;
    }

    if (remove_event) {
        LOG_DEBUG ("EVENT REMOVED (id=%d)", event->policy_id);
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

    if ((def = _event_manager_get_definition (self, event_name)) == NULL) {
        LOG_ERROR ("No event definition for event %s", event_name);
        goto failed;
    }

    /* Then, get the play mode from the passed properties. The play mode is either "long"
       or "short" and we have a corresponding prototypes defined in the event definition.

       If no play mode then this is an invalid event. */

    if ((play_mode = _properties_get_play_mode (properties)) == 0) {
        LOG_ERROR ("No play.mode property for event %s", event_name);
        goto failed;
    }

    /* Lookup the prototype based on the play mode. If not found, we have the definition,
       but no actions specified for the play mode. */

    proto_name = (play_mode == NGF_PLAY_MODE_LONG) ? def->long_proto : def->short_proto;

    if ((proto = ngf_daemon_get_prototype (self, proto_name)) == 0) {
        LOG_ERROR ("Failed to get event prototype %s for event %s", proto_name, event_name);
        goto failed;
    }

    /* Get the policy identifier, allowed resources and play mode and timeout
       for our event. */

    policy_id    = _properties_get_policy_id (properties);
    play_timeout = _properties_get_play_timeout (properties);
    resources    = _properties_get_resources (properties);

    if (policy_id == 0 || resources == 0) {
        LOG_ERROR ("No policy.id or resources defined for event %s", event_name);
        goto failed;
    }

    LOG_EVENT ("event_name=%s, proto_name=%s, policy_id=%d, play_timeout=%d, resources=0x%X, play_mode=%d (%s))",
        event_name, proto_name, policy_id, play_timeout, resources, play_mode, play_mode == NGF_PLAY_MODE_LONG ? "LONG" : "SHORT");

    /* Create a new event based on the prototype and feed the properties
       to it. */

    if ((event = ngf_event_new (&self->context, proto)) == NULL) {
        LOG_ERROR ("Failed to create event %s", event_name);
        goto failed;
    }

    event->policy_id    = policy_id;
    event->resources    = resources;
    event->play_mode    = play_mode;
    event->play_timeout = play_timeout;

    /* Setup the event callback to monitor events progress. */

    ngf_event_set_callback (event, _event_state_cb, self);

    /* Start the event immediately. */

    self->event_list = g_list_append (self->event_list, event);
    if (!ngf_event_start (event, properties)) {
        LOG_ERROR ("Failed to start event %s", event_name);
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
            LOG_DEBUG ("EVENT STOP (id=%d)\n", id);
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
