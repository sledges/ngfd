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

static gboolean     _request_manager_create         (Context *context);
static void         _request_manager_destroy        (Context *context);
EventDefinition*    _request_manager_get_definition (Context *context, const char *name);
static void         _request_state_cb               (Request *request, guint state, gpointer userdata);
static gboolean     _properties_get_boolean         (GHashTable *properties, const char *key);
static guint        _properties_get_policy_id       (GHashTable *properties);
static guint        _properties_get_play_timeout    (GHashTable *properties);
static gint         _properties_get_play_mode       (GHashTable *properties);
static gint         _properties_get_resources       (GHashTable *properties);

static DBusConnection*
_get_dbus_connection (DBusBusType bus_type)
{
    DBusConnection *bus = NULL;
    DBusError       error;

    dbus_error_init (&error);
    bus = dbus_bus_get (bus_type, &error);
    if (bus == NULL) {
        LOG_WARNING ("Failed to get %s bus: %s", bus_type == DBUS_BUS_SYSTEM ? "system" : "session", error.message);
        dbus_error_free (&error);
        return NULL;
    }

    return bus;
}

Context*
daemon_create ()
{
    Context *context = NULL;

    if ((context = g_new0 (Context, 1)) == NULL) {
        LOG_ERROR ("Failed to allocate memory for Context instance!");
        return NULL;
    }

    if ((context->loop = g_main_loop_new (NULL, 0)) == NULL) {
        LOG_ERROR ("Failed to create the GLib mainloop!");
        return NULL;
    }

    /* setup the dbus connections. we will hook up to the session bus, but use
       the system bus too for led, backlight and tone generator. */

    context->system_bus  = _get_dbus_connection (DBUS_BUS_SYSTEM);
    context->session_bus = _get_dbus_connection (DBUS_BUS_SESSION);

    if (!context->system_bus || !context->session_bus) {
        LOG_ERROR ("Failed to get system/session bus!");
        return NULL;
    }

    dbus_connection_setup_with_g_main (context->system_bus, NULL);
    dbus_connection_setup_with_g_main (context->session_bus, NULL);

    if (!dbus_if_create (context)) {
        LOG_ERROR ("Failed to create D-Bus interface!");
        return NULL;
    }

    if ((context->profile = profile_create ()) == NULL) {
        LOG_ERROR ("Failed to create profile tracking!");
        return NULL;
    }

    if ((context->tone_mapper = tone_mapper_create ()) == NULL) {
        LOG_WARNING ("Failed to create tone mapper!");
    }

    if ((context->audio = audio_create ()) == NULL) {
        LOG_ERROR ("Failed to create Pulseaudio backend!");
        return NULL;
    }

    if ((context->vibrator = vibrator_create ()) == NULL) {
        LOG_ERROR ("Failed to create Immersion backend!");
        return NULL;
    }

    if ((context->tonegen = tone_generator_create ()) == NULL) {
        LOG_ERROR ("Failed to create tone generator backend!");
        return NULL;
    }

    if ((context->led = led_create ()) == NULL) {
        LOG_ERROR ("Failed to create LED backend!");
        return NULL;
    }

    if ((context->backlight = backlight_create ()) == NULL) {
        LOG_ERROR ("Failed to create backlight backend!");
        return NULL;
    }

    if (!_request_manager_create (context)) {
        LOG_ERROR ("Failed to create request manager!");
        return NULL;
    }

    if (!daemon_settings_load (context)) {
        LOG_ERROR ("Failed to load settings!");
        return NULL;
    }

    return context;
}

void
daemon_destroy (Context *context)
{
    dbus_if_destroy (context);

    if (context->session_bus) {
        dbus_connection_unref (context->session_bus);
        context->session_bus = NULL;
    }

    if (context->system_bus) {
        dbus_connection_unref (context->system_bus);
        context->system_bus = NULL;
    }

    if (context->backlight) {
        backlight_destroy (context->backlight);
	    context->backlight = NULL;
    }

    if (context->led) {
        led_destroy (context->led);
        context->led = NULL;
    }

    if (context->tonegen) {
        tone_generator_destroy (context->tonegen);
        context->tonegen = NULL;
    }

    if (context->vibrator) {
        vibrator_destroy (context->vibrator);
        context->vibrator = NULL;
    }

    if (context->audio) {
        audio_destroy (context->audio);
        context->audio = NULL;
    }

    if (context->tone_mapper) {
        tone_mapper_destroy (context->tone_mapper);
        context->tone_mapper = NULL;
    }

    if (context->profile) {
        profile_destroy (context->profile);
        context->profile = NULL;
    }

    _request_manager_destroy (context);

    if (context->loop) {
        g_main_loop_unref (context->loop);
        context->loop = NULL;
    }

    g_free (context);
}

void
daemon_run (Context *context)
{
    g_main_loop_run (context->loop);
}

static gboolean
_request_manager_create (Context *context)
{
    context->definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) event_definition_free);
    if (context->definitions == NULL)
        return FALSE;

    context->prototypes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) event_prototype_free);
    if (context->prototypes == NULL)
        return FALSE;

    return TRUE;
}

static void
_request_manager_destroy (Context *context)
{
    if (context->prototypes) {
        g_hash_table_destroy (context->prototypes);
        context->prototypes = NULL;
    }

    if (context->definitions) {
        g_hash_table_destroy (context->definitions);
        context->definitions = NULL;
    }
}

void
daemon_register_definition (Context *context, const char *name, EventDefinition *def)
{
    if (context == NULL || name == NULL || def == NULL)
        return;

    g_hash_table_replace (context->definitions, g_strdup (name), def);
}

EventDefinition*
_request_manager_get_definition (Context *context, const char *name)
{
    if (context == NULL || name == NULL)
        return NULL;

    return (EventDefinition*) g_hash_table_lookup (context->definitions, name);
}

void
daemon_register_prototype (Context *context, const char *name, EventPrototype *proto)
{
    if (context == NULL || name == NULL || proto == NULL)
        return;

    g_hash_table_replace (context->prototypes, g_strdup (name), proto);
}

EventPrototype*
daemon_get_prototype (Context *context, const char *name)
{
    if (context == NULL || name == NULL)
        return NULL;

    return (EventPrototype*) g_hash_table_lookup (context->prototypes, name);
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
_request_state_cb (Request *request, guint state, gpointer userdata)
{
    Context *context = (Context*) userdata;
    gboolean remove_request = FALSE;

    switch (state) {
        case REQUEST_STATE_STARTED:
            TIMESTAMP ("REQUEST started");
            LOG_DEBUG ("REQUEST STARTED (id=%d): Time: %f s", request->policy_id, g_timer_elapsed (request->start_timer,NULL));
            break;

        case REQUEST_STATE_FAILED:
            LOG_DEBUG ("REQUEST FAILED (id=%d)", request->policy_id);
            dbus_if_send_status (context, request->policy_id, 0);
            remove_request = TRUE;
            break;

        case REQUEST_STATE_COMPLETED:
            LOG_DEBUG ("REQUEST COMPLETED (id=%d)", request->policy_id);
            dbus_if_send_status (context, request->policy_id, 0);
            remove_request = TRUE;
            break;

        default:
            break;
    }

    if (remove_request) {
        LOG_DEBUG ("REQUEST REMOVED (id=%d)", request->policy_id);
        context->request_list = g_list_remove (context->request_list, request);
        request_stop (request);
        request_free (request);
    }
}

guint
daemon_request_play (Context *context, const char *request_name, GHashTable *properties)
{
    EventDefinition *def = NULL;
    EventPrototype *proto = NULL;
    Request *request = NULL;

    guint policy_id = 0, play_timeout = 0;
    gint resources = 0, play_mode = 0;

    const char *proto_name      = NULL;
    const char *current_profile = NULL;

    TIMESTAMP ("Request request received");

    /* First, look for the request definition that defines the prototypes for long and
       short requests. If not found, then it is an unrecognized request. */

    if ((def = _request_manager_get_definition (context, request_name)) == NULL) {
        LOG_ERROR ("No request definition for request %s", request_name);
        return 0;
    }

    /* Then, get the play mode from the passed properties. The play mode is either "long"
       or "short" and we have a corresponding prototypes defined in the request definition.

       If no play mode then this is an invalid request. */

    if ((play_mode = _properties_get_play_mode (properties)) == 0) {
        LOG_ERROR ("No play.mode property for request %s", request_name);
        return 0;
    }

    /* If the current profile is meeting and meeting profile prototype is set in the
       definition, use that. Otherwise, lookup the prototype based on the play mode.
       If not found, we have the definition, but no actions specified for the play mode. */

    current_profile = profile_get_current (context->profile);
    if (def->meeting_proto && current_profile && g_str_equal (current_profile, PROFILE_MEETING)) {
        proto_name = def->meeting_proto;
    }
    else {
        proto_name = (play_mode == PLAY_MODE_LONG) ? def->long_proto : def->short_proto;
    }

    if ((proto = daemon_get_prototype (context, proto_name)) == 0) {
        LOG_ERROR ("Failed to get request prototype %s for request %s", proto_name, request_name);
        return 0;
    }

    /* Get the policy identifier, allowed resources and play mode and timeout
       for our request. */

    policy_id    = _properties_get_policy_id (properties);
    play_timeout = _properties_get_play_timeout (properties);
    resources    = _properties_get_resources (properties);

    if (policy_id == 0 || resources == 0) {
        LOG_ERROR ("No policy.id or resources defined for request %s", request_name);
        return 0;
    }

    LOG_REQUEST ("request_name=%s, proto_name=%s, policy_id=%d, play_timeout=%d, resources=0x%X, play_mode=%d (%s))",
        request_name, proto_name, policy_id, play_timeout, resources, play_mode, play_mode == PLAY_MODE_LONG ? "LONG" : "SHORT");

    TIMESTAMP ("Request parsing completed");
    /* Create a new request based on the prototype and feed the properties
       to it. */

    if ((request = request_new (context, proto)) == NULL) {
        LOG_ERROR ("Failed to create request %s", request_name);
        return 0;
    }

    request->policy_id    = policy_id;
    request->resources    = resources;
    request->play_mode    = play_mode;
    request->play_timeout = play_timeout;

    /* Setup the request callback to monitor requests progress. */

    request_set_callback (request, _request_state_cb, context);

    /* Start the request immediately. */

    context->request_list = g_list_append (context->request_list, request);
    if (!request_start (request, properties)) {
        LOG_ERROR ("Failed to start request %s", request_name);
        context->request_list = g_list_remove (context->request_list, request);
        request_free (request);
        return 0;
    }

    return 1;
}

void
daemon_request_stop (Context *context, guint id)
{
    Request *request = NULL;
    GList *iter = NULL;

    if (context->request_list == NULL)
        return;

    for (iter = g_list_first (context->request_list); iter; iter = g_list_next (context->request_list)) {
        request = (Request*) iter->data;
        if (request->policy_id == id) {
            LOG_DEBUG ("request STOP (id=%d)\n", id);
            context->request_list = g_list_remove (context->request_list, request);
            request_stop (request);
            dbus_if_send_status (context, request->policy_id, 0);
            request_free (request);
            break;
        }
    }
}
