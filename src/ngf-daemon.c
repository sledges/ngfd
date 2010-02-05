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
#include "ngf-conf.h"
#include "ngf-event.h"
#include "ngf-daemon.h"

static gboolean _configuration_load          (NgfDaemon *self);
static void     _audio_state_cb              (NgfAudio *audio, NgfAudioState state, gpointer userdata);
static void     _event_state_cb              (NgfEvent *event, NgfEventState state, gpointer userdata);
static gboolean _properties_get_boolean      (GHashTable *properties, const char *key);
static guint    _properties_get_policy_id    (GHashTable *properties);
static guint    _properties_get_play_timeout (GHashTable *properties);
static gint     _properties_get_play_mode    (GHashTable *properties);
static gint     _properties_get_resources    (GHashTable *properties);
static guint    _handle_play_cb              (NgfDBus *dbus, const char *sender, const char *event, GHashTable *properties, gpointer userdata);
static void     _handle_stop_cb              (NgfDBus *dbus, guint id, gpointer userdata);

static void
_configuration_parse_general (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    gboolean log_events;

    ngf_conf_get_boolean (c, group, "log_events", &log_events, FALSE);
}

static void
_configuration_parse_event (NgfConf *c, const char *group, const char *name, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;

    if (name == NULL)
        return;

    NgfEventPrototype *proto = NULL;
    proto = ngf_event_prototype_new ();

    ngf_conf_get_boolean (c, group, "repeat", &proto->event_repeat, FALSE);
    ngf_conf_get_integer (c, group, "max_length", &proto->event_max_length, 0);

    ngf_conf_get_string (c, group, "audio_filename", &proto->audio_filename, NULL);
    ngf_conf_get_string (c, group, "audio_fallback", &proto->audio_fallback, NULL);

    ngf_conf_get_string (c, group, "audio_tone_key", &proto->audio_tone_key, NULL);
    ngf_conf_get_string (c, group, "audio_fallback_key", &proto->audio_fallback_key, NULL);
    ngf_conf_get_string (c, group, "audio_volume_key", &proto->audio_volume_key, NULL);
    ngf_conf_get_string (c, group, "audio_stream_restore", &proto->audio_stream_restore, NULL);

    g_print ("Registering event prototype: %s\n", name);
    ngf_event_manager_register_prototype (self->event_manager, name, proto);
}

static gboolean
_configuration_load (NgfDaemon *self)
{
    static const char *conf_files[] = { "/etc/ngf/ngf.ini", "./ngf.ini", NULL };

    NgfConf *conf = NULL;
    const char **file = NULL;
    gboolean success = FALSE;

    conf = ngf_conf_new ();
    ngf_conf_add_group (conf, NGF_CONF_PARSE_EXACT, "general", _configuration_parse_general, self);
    ngf_conf_add_group (conf, NGF_CONF_PARSE_PREFIX, "event", _configuration_parse_event, self);

    for (file = conf_files; *file; file++) {
        if (g_file_test (*file, G_FILE_TEST_EXISTS)) {
            g_print ("trying to load = %s\n", *file);
            if (ngf_conf_load (conf, *file))
                success = TRUE;
            break;
        }
    }

    ngf_conf_free (conf);

    return success;
}

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

    if ((self->context.audio = ngf_audio_create ()) == NULL)
        return NULL;

    if ((self->context.vibrator = ngf_vibrator_create ()) == NULL)
        return NULL;

    if (!_configuration_load (self))
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

    if (self->context.vibrator) {
        ngf_vibrator_destroy (self->context.vibrator);
        self->context.vibrator = NULL;
    }

    if (self->context.audio) {
        ngf_audio_destroy (self->context.audio);
        self->context.audio = NULL;
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
            remove_event = TRUE;
            break;

        case NGF_EVENT_COMPLETED:
            g_print ("EVENT COMPLETED (id=%d)\n", event->policy_id);
            remove_event = TRUE;
            break;

        default:
            break;
    }

    if (remove_event) {
        self->event_list = g_list_remove (self->event_list, event);
        ngf_event_free (event);
    }
}

guint
ngf_daemon_event_play (NgfDaemon *self, const char *sender, const char *event_name, GHashTable *properties)
{
    NgfEventPrototype *proto = NULL;
    NgfEvent *event = NULL;

    guint policy_id = 0, play_timeout = 0;
    gint resources = 0, play_mode = 0;

    /* First, we will try to lookup if we have an event prototype that matches
       the event name. If not, then we will ignore this request straight on
       since it is unknown to us. */

    if ((proto = ngf_event_manager_get_prototype (self->event_manager, event_name)) == 0)
        return 0;

    /* Get the policy identifier, allowed resources and play mode and timeout
       for our event. */

    policy_id    = _properties_get_policy_id (properties);
    play_timeout = _properties_get_play_timeout (properties);
    resources    = _properties_get_resources (properties);
    play_mode    = _properties_get_play_mode (properties);

    if (policy_id == 0 || resources == 0 || play_mode == 0)
        return 0;

    /* Create a new event based on the prototype and feed the properties
       to it. */

    if ((event = ngf_event_new (&self->context, proto)) == NULL)
        return 0;

    event->properties   = properties;
    event->policy_id    = policy_id;
    event->resources    = resources;
    event->play_mode    = play_mode;
    event->play_timeout = play_timeout;

    /* Setup the event callback to monitor events progress. */

    ngf_event_set_callback (event, _event_state_cb, self);

    /* Start the event immediately. */

    self->event_list = g_list_append (self->event_list, event);
    if (!ngf_event_start (event)) {
        self->event_list = g_list_remove (self->event_list, event);
        ngf_event_free (event);
        return 0;
    }

    return 1;
}

void
ngf_daemon_event_stop (NgfDaemon *self, guint id)
{
    NgfEvent *event = NULL;
    GList *iter = NULL;

    for (iter = g_list_first (self->event_list); iter; iter = g_list_next (self->event_list)) {
        event = (NgfEvent*) iter->data;
        if (event->policy_id == id) {
            self->event_list = g_list_remove (self->event_list, event);
            ngf_event_stop (event);
            ngf_event_free (event);
            break;
        }
    }
}

static guint
_handle_play_cb (NgfDBus *dbus, const char *sender, const char *event, GHashTable *properties, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    return ngf_daemon_event_play (self, sender, event, properties);
}

static void
_handle_stop_cb (NgfDBus *dbus, guint id, gpointer userdata)
{
    NgfDaemon *self = (NgfDaemon*) userdata;
    ngf_daemon_event_stop (self, id);
}
