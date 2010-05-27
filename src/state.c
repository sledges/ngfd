#include "log.h"
#include "timestamp.h"
#include "resources.h"
#include "request.h"
#include "property.h"
#include "player.h"
#include "state.h"

static gboolean _properties_get_boolean         (GHashTable *properties, const char *key);
static guint    _properties_get_policy_id       (GHashTable *properties);
static guint    _properties_get_play_timeout    (GHashTable *properties);
static gint     _properties_get_play_mode       (GHashTable *properties);
static gint     _properties_get_resources       (GHashTable *properties);
static void     _request_state_cb               (Request *request, guint state, gpointer userdata);

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

static const gchar*
_properties_get_string (GHashTable *properties, const char *key)
{
    Property *value = NULL;

    if ((value = g_hash_table_lookup (properties, key)) == NULL)
        return NULL;

    if (property_get_type (value) == PROPERTY_TYPE_STRING)
        return property_get_string (value);

    return NULL;
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

    if ((str = _properties_get_string (properties, "play.mode")) == NULL)
        return 0;

    if (g_str_equal (str, "short"))
        return REQUEST_PLAY_MODE_SHORT;
    else if (g_str_equal (str, "long"))
        return REQUEST_PLAY_MODE_LONG;

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
        resources |= RESOURCE_LEDS;

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
            TIMESTAMP ("REQUEST STARTED");
            LOG_DEBUG ("request (%d) >> started", request->policy_id);
            break;

        case REQUEST_STATE_FAILED:
            LOG_DEBUG ("request (%d) >> failed", request->policy_id);
            dbus_if_send_status (context, request->policy_id, 0);
            remove_request = TRUE;
            break;

        case REQUEST_STATE_UPDATED:
            LOG_DEBUG ("request (%d) >> resource update", request->policy_id);
            dbus_if_send_resource_update (context, request->policy_id, TRUE, TRUE, FALSE, TRUE);
            break;

        case REQUEST_STATE_COMPLETED:
            LOG_DEBUG ("request (%d) >> completed", request->policy_id);
            dbus_if_send_status (context, request->policy_id, 0);
            remove_request = TRUE;
            break;

        default:
            break;
    }

    if (remove_request) {
        LOG_DEBUG ("request (%d) >> removed", request->policy_id);

        context->request_list = g_list_remove (context->request_list, request);
        stop_request (request);
        request_free (request);
    }
}

guint
play_handler (Context *context, const char *request_name, GHashTable *properties)
{
    Definition *def     = NULL;
    Event      *event   = NULL;
    Request    *request = NULL;

    guint policy_id    = 0;
    guint play_timeout = 0;
    gint  resources    = 0;
    gint  play_mode    = 0;

    const char *event_name      = NULL;
    const char *current_profile = NULL;

    TIMESTAMP ("Request request received");

    /* First, look for the request definition that defines the events for long and
       short requests. If not found, then it is an unrecognized request. */

    if ((def = g_hash_table_lookup (context->definitions, request_name)) == NULL) {
        LOG_ERROR ("No request definition for request %s", request_name);
        return 0;
    }

    /* Then, get the play mode from the passed properties. The play mode is either "long"
       or "short" and we have a corresponding events defined in the request definition.
       If no play mode then this is an invalid request. */

    if ((play_mode = _properties_get_play_mode (properties)) == 0) {
        LOG_ERROR ("No play.mode property for request %s", request_name);
        return 0;
    }

    /* If the current profile is meeting and meeting profile event is set in the
       definition, use that. Otherwise, lookup the event based on the play mode.
       If not found, we have the definition, but no actions specified for the play mode. */

    if (def->meeting_event && context->meeting_mode) {
        event_name = def->meeting_event;
    }
    else {
        event_name = (play_mode == REQUEST_PLAY_MODE_LONG) ? def->long_event : def->short_event;
    }

    if ((event = g_hash_table_lookup (context->events, event_name)) == 0) {
        LOG_ERROR ("Failed to get request event %s for request %s", event_name, request_name);
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

    LOG_REQUEST ("request_name=%s, event_name=%s, policy_id=%d, play_timeout=%d, resources=0x%X, play_mode=%d (%s))",
        request_name, event_name, policy_id, play_timeout, resources, play_mode, play_mode == REQUEST_PLAY_MODE_LONG ? "LONG" : "SHORT");

    TIMESTAMP ("Request parsing completed");

    /* Create a new request based on the event and feed the properties
       to it. */

    if ((request = request_new (context, event)) == NULL) {
        LOG_WARNING ("request (%d) >> failed create request", policy_id);
        return 0;
    }

    request->policy_id    = policy_id;
    request->resources    = resources;
    request->play_mode    = play_mode;
    request->play_timeout = play_timeout;
    request->callback     = _request_state_cb;
    request->userdata     = context;

    /* set the user provided custom sound, if any and
       start the playback of the request. */

    if (event->allow_custom)
        request_set_custom_sound (request, _properties_get_string (properties, "audio"));

    context->request_list = g_list_append (context->request_list, request);
    if (!play_request (request)) {
        LOG_WARNING ("request (%d) >> failed to start", request->policy_id);
        context->request_list = g_list_remove (context->request_list, request);
        request_free (request);
        return 0;
    }

    return 1;
}

void
stop_handler (Context *context, guint policy_id)
{
    Request *request = NULL;
    GList *iter = NULL;

    if (context->request_list == NULL)
        return;

    for (iter = g_list_first (context->request_list); iter; iter = g_list_next (context->request_list)) {
        request = (Request*) iter->data;
        if (request->policy_id == policy_id) {
            LOG_DEBUG ("request (%d) >> stop received", policy_id);

            context->request_list = g_list_remove (context->request_list, request);

            stop_request (request);
            request_free (request);

            dbus_if_send_status (context, policy_id, 0);
            break;
        }
    }
}
