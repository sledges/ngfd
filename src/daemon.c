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

#include "log.h"
#include "property.h"
#include "timestamp.h"
#include "daemon.h"

static gboolean     _event_manager_create         (Daemon *self);
static void         _event_manager_destroy        (Daemon *self);
EventDefinition* _event_manager_get_definition (Daemon *self, const char *name);
static void         _event_state_cb               (Event *event, EventState state, gpointer userdata);
static gboolean     _properties_get_boolean       (GHashTable *properties, const char *key);
static guint        _properties_get_policy_id     (GHashTable *properties);
static guint        _properties_get_play_timeout  (GHashTable *properties);
static gint         _properties_get_play_mode     (GHashTable *properties);
static gint         _properties_get_resources     (GHashTable *properties);
static guint        _handle_play_cb               (DBusIf *dbus, const char *event, GHashTable *properties, gpointer userdata);
static void         _handle_stop_cb               (DBusIf *dbus, guint id, gpointer userdata);

Daemon*
daemon_create ()
{
    Daemon *self = NULL;

    if ((self = g_new0 (Daemon, 1)) == NULL) {
        LOG_ERROR ("Failed to allocate memory for Daemon instance!");
        return NULL;
    }

    if ((self->loop = g_main_loop_new (NULL, 0)) == NULL) {
        LOG_ERROR ("Failed to create the GLib mainloop!");
        return NULL;
    }

    if ((self->dbus = dbus_if_create (_handle_play_cb, _handle_stop_cb, self)) == NULL) {
        LOG_ERROR ("Failed to create D-Bus interface!");
        return NULL;
    }

    if ((self->context.profile = profile_create ()) == NULL) {
        LOG_ERROR ("Failed to create profile tracking!");
        return NULL;
    }

    if ((self->context.tone_mapper = tone_mapper_create ()) == NULL) {
        LOG_WARNING ("Failed to create tone mapper!");
    }

    if ((self->context.audio = audio_create ()) == NULL) {
        LOG_ERROR ("Failed to create Pulseaudio backend!");
        return NULL;
    }

    if ((self->context.vibrator = vibrator_create ()) == NULL) {
        LOG_ERROR ("Failed to create Immersion backend!");
        return NULL;
    }

    if ((self->context.tonegen = tone_generator_create ()) == NULL) {
        LOG_ERROR ("Failed to create tone generator backend!");
        return NULL;
    }

    if ((self->context.led = led_create ()) == NULL) {
        LOG_ERROR ("Failed to create LED backend!");
        return NULL;
    }

    if ((self->context.backlight = backlight_create ()) == NULL) {
        LOG_ERROR ("Failed to create backlight backend!");
        return NULL;
    }

    if (!_event_manager_create (self)) {
        LOG_ERROR ("Failed to create event manager!");
        return NULL;
    }

    if (!daemon_settings_load (self)) {
        LOG_ERROR ("Failed to load settings!");
        return NULL;
    }

    return self;
}

void
daemon_destroy (Daemon *self)
{
    if (self->dbus) {
        dbus_if_destroy (self->dbus);
        self->dbus = NULL;
    }

    if (self->context.backlight) {
        backlight_destroy (self->context.backlight);
	    self->context.backlight = NULL;
    }

    if (self->context.led) {
        led_destroy (self->context.led);
        self->context.led = NULL;
    }

    if (self->context.tonegen) {
        tone_generator_destroy (self->context.tonegen);
        self->context.tonegen = NULL;
    }

    if (self->context.vibrator) {
        vibrator_destroy (self->context.vibrator);
        self->context.vibrator = NULL;
    }

    if (self->context.audio) {
        audio_destroy (self->context.audio);
        self->context.audio = NULL;
    }

    if (self->context.tone_mapper) {
        tone_mapper_destroy (self->context.tone_mapper);
        self->context.tone_mapper = NULL;
    }

    if (self->context.profile) {
        profile_destroy (self->context.profile);
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
daemon_run (Daemon *self)
{
    g_main_loop_run (self->loop);
}

static gboolean
_event_manager_create (Daemon *self)
{
    self->definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) event_definition_free);
    if (self->definitions == NULL)
        return FALSE;

    self->prototypes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) event_prototype_free);
    if (self->prototypes == NULL)
        return FALSE;

    return TRUE;
}

static void
_event_manager_destroy (Daemon *self)
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
daemon_register_definition (Daemon *self, const char *name, EventDefinition *def)
{
    if (self == NULL || name == NULL || def == NULL)
        return;

    g_hash_table_replace (self->definitions, g_strdup (name), def);
}

EventDefinition*
_event_manager_get_definition (Daemon *self, const char *name)
{
    if (self == NULL || name == NULL)
        return NULL;

    return (EventDefinition*) g_hash_table_lookup (self->definitions, name);
}

void
daemon_register_prototype (Daemon *self, const char *name, EventPrototype *proto)
{
    if (self == NULL || name == NULL || proto == NULL)
        return;

    g_hash_table_replace (self->prototypes, g_strdup (name), proto);
}

EventPrototype*
daemon_get_prototype (Daemon *self, const char *name)
{
    if (self == NULL || name == NULL)
        return NULL;

    return (EventPrototype*) g_hash_table_lookup (self->prototypes, name);
}

static gboolean
_properties_get_boolean (GHashTable *properties, const char *key)
{
    Property *value = NULL;

    if ((value = g_hash_table_lookup (properties, key)) == NULL)
        return FALSE;

    if (property_get_type (value) == PROPERTY_TYPE_BOOLEAN)
        return property_get_boolean (value);

    return FALSE;
}

static guint
_properties_get_policy_id (GHashTable *properties)
{
    Property *value = NULL;

    if ((value = g_hash_table_lookup (properties, "policy.id")) == NULL)
        return 0;

    if (property_get_type (value) == PROPERTY_TYPE_UINT)
        return property_get_uint (value);

    return 0;
}

static guint
_properties_get_play_timeout (GHashTable *properties)
{
    Property *value = NULL;

    if ((value = g_hash_table_lookup (properties, "play.timeout")) == NULL)
        return 0;

    if (property_get_type (value) == PROPERTY_TYPE_UINT)
        return property_get_uint (value);

    return 0;
}

static gint
_properties_get_play_mode (GHashTable *properties)
{
    Property *value = NULL;
    const char *str = NULL;

    if ((value = g_hash_table_lookup (properties, "play.mode")) == NULL)
        return 0;

    if (property_get_type (value) == PROPERTY_TYPE_STRING) {
        str = property_get_string (value);

        if (g_str_equal (str, "short"))
            return PLAY_MODE_SHORT;
        else if (g_str_equal (str, "long"))
            return PLAY_MODE_LONG;
    }

    return 0;
}

static gint
_properties_get_resources (GHashTable *properties)
{
    gint resources = 0;

    if (_properties_get_boolean (properties, "media.audio"))
        resources |= RESOURCE_AUDIO;

    if (_properties_get_boolean (properties, "media.vibra"))
        resources |= RESOURCE_VIBRATION;

    if (_properties_get_boolean (properties, "media.leds"))
        resources |= RESOURCE_LED;

    if (_properties_get_boolean (properties, "media.backlight"))
        resources |= RESOURCE_BACKLIGHT;

    return resources;
}

static void
_event_state_cb (Event *event, EventState state, gpointer userdata)
{
    Daemon *self = (Daemon*) userdata;
    gboolean remove_event = FALSE;

    switch (state) {
        case EVENT_STARTED:
            TIMESTAMP ("Event started");
            LOG_DEBUG ("EVENT STARTED (id=%d): Time: %f s", event->policy_id, g_timer_elapsed(event->start_timer,NULL));
            break;

        case EVENT_FAILED:
            LOG_DEBUG ("EVENT FAILED (id=%d)", event->policy_id);
            dbus_if_send_status (self->dbus, event->policy_id, 0);
            remove_event = TRUE;
            break;

        case EVENT_COMPLETED:
            LOG_DEBUG ("EVENT COMPLETED (id=%d)", event->policy_id);
            dbus_if_send_status (self->dbus, event->policy_id, 0);
            remove_event = TRUE;
            break;

        default:
            break;
    }

    if (remove_event) {
        LOG_DEBUG ("EVENT REMOVED (id=%d)", event->policy_id);
        self->event_list = g_list_remove (self->event_list, event);
        event_stop (event);
        event_free (event);
    }
}

guint
daemon_event_play (Daemon *self, const char *event_name, GHashTable *properties)
{
    EventDefinition *def = NULL;
    EventPrototype *proto = NULL;
    Event *event = NULL;

    guint policy_id = 0, play_timeout = 0;
    gint resources = 0, play_mode = 0;

    const char *proto_name      = NULL;
    const char *current_profile = NULL;

    TIMESTAMP ("Event request received");

    /* First, look for the event definition that defines the prototypes for long and
       short events. If not found, then it is an unrecognized event. */

    if ((def = _event_manager_get_definition (self, event_name)) == NULL) {
        LOG_ERROR ("No event definition for event %s", event_name);
        return 0;
    }

    /* Then, get the play mode from the passed properties. The play mode is either "long"
       or "short" and we have a corresponding prototypes defined in the event definition.

       If no play mode then this is an invalid event. */

    if ((play_mode = _properties_get_play_mode (properties)) == 0) {
        LOG_ERROR ("No play.mode property for event %s", event_name);
        return 0;
    }

    /* If the current profile is meeting and meeting profile prototype is set in the
       definition, use that. Otherwise, lookup the prototype based on the play mode.
       If not found, we have the definition, but no actions specified for the play mode. */

    current_profile = profile_get_current (self->context.profile);
    if (def->meeting_proto && current_profile && g_str_equal (current_profile, PROFILE_MEETING)) {
        proto_name = def->meeting_proto;
    }
    else {
        proto_name = (play_mode == PLAY_MODE_LONG) ? def->long_proto : def->short_proto;
    }

    if ((proto = daemon_get_prototype (self, proto_name)) == 0) {
        LOG_ERROR ("Failed to get event prototype %s for event %s", proto_name, event_name);
        return 0;
    }

    /* Get the policy identifier, allowed resources and play mode and timeout
       for our event. */

    policy_id    = _properties_get_policy_id (properties);
    play_timeout = _properties_get_play_timeout (properties);
    resources    = _properties_get_resources (properties);

    if (policy_id == 0 || resources == 0) {
        LOG_ERROR ("No policy.id or resources defined for event %s", event_name);
        return 0;
    }

    LOG_EVENT ("event_name=%s, proto_name=%s, policy_id=%d, play_timeout=%d, resources=0x%X, play_mode=%d (%s))",
        event_name, proto_name, policy_id, play_timeout, resources, play_mode, play_mode == PLAY_MODE_LONG ? "LONG" : "SHORT");

    TIMESTAMP ("Event parsing completed");
    /* Create a new event based on the prototype and feed the properties
       to it. */

    if ((event = event_new (&self->context, proto)) == NULL) {
        LOG_ERROR ("Failed to create event %s", event_name);
        return 0;
    }

    event->policy_id    = policy_id;
    event->resources    = resources;
    event->play_mode    = play_mode;
    event->play_timeout = play_timeout;

    /* Setup the event callback to monitor events progress. */

    event_set_callback (event, _event_state_cb, self);

    /* Start the event immediately. */

    self->event_list = g_list_append (self->event_list, event);
    if (!event_start (event, properties)) {
        LOG_ERROR ("Failed to start event %s", event_name);
        self->event_list = g_list_remove (self->event_list, event);
        event_free (event);
        return 0;
    }

    return 1;
}

void
daemon_event_stop (Daemon *self, guint id)
{
    Event *event = NULL;
    GList *iter = NULL;

    if (self->event_list == NULL)
        return;

    for (iter = g_list_first (self->event_list); iter; iter = g_list_next (self->event_list)) {
        event = (Event*) iter->data;
        if (event->policy_id == id) {
            LOG_DEBUG ("EVENT STOP (id=%d)\n", id);
            self->event_list = g_list_remove (self->event_list, event);
            event_stop (event);
            dbus_if_send_status (self->dbus, event->policy_id, 0);
            event_free (event);
            break;
        }
    }
}

static guint
_handle_play_cb (DBusIf *dbus, const char *event, GHashTable *properties, gpointer userdata)
{
    Daemon *self = (Daemon*) userdata;
    return daemon_event_play (self, event, properties);
}

static void
_handle_stop_cb (DBusIf *dbus, guint id, gpointer userdata)
{
    Daemon *self = (Daemon*) userdata;
    daemon_event_stop (self, id);
}
