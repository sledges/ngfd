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

#include "log.h"
#include "timestamp.h"
#include "resources.h"
#include "request.h"
#include "proplist.h"
#include "player.h"
#include "dbus-if.h"
#include "state.h"

static guint    _properties_get_policy_id       (NProplist *proplist);
static guint    _properties_get_play_timeout    (NProplist *proplist);
static gint     _properties_get_play_mode       (NProplist *proplist);
static gint     _properties_get_resources       (NProplist *proplist);
static void     _request_state_cb               (Request *request, guint state, gpointer userdata);

static guint
_properties_get_policy_id (NProplist *proplist)
{
    return n_proplist_get_uint (proplist, "policy.id");
}

static guint
_properties_get_play_timeout (NProplist *proplist)
{
    return n_proplist_get_uint (proplist, "play.timeout");
}

static gint
_properties_get_play_mode (NProplist *proplist)
{
    const char *str = NULL;

    if ((str = n_proplist_get_string (proplist, "play.mode")) == NULL)
        return 0;

    if (g_str_equal (str, "short"))
        return REQUEST_PLAY_MODE_SHORT;
    else if (g_str_equal (str, "long"))
        return REQUEST_PLAY_MODE_LONG;

    return 0;
}

static gint
_properties_get_resources (NProplist *proplist)
{
    gint resources = 0;

    if (n_proplist_get_bool (proplist, "media.audio"))
        resources |= RESOURCE_AUDIO;

    if (n_proplist_get_bool (proplist, "media.vibra"))
        resources |= RESOURCE_VIBRATION;

    if (n_proplist_get_bool (proplist, "media.leds"))
        resources |= RESOURCE_LEDS;

    if (n_proplist_get_bool (proplist, "media.backlight"))
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
            N_INFO ("request (%d) >> started", request->policy_id);
            break;

        case REQUEST_STATE_FAILED:
            N_DEBUG ("request (%d) >> failed", request->policy_id);
            dbus_if_send_status (context, request->policy_id, NGF_STATUS_FAILED);
            remove_request = TRUE;
            break;

        case REQUEST_STATE_COMPLETED:
            N_INFO ("request (%d) >> completed", request->policy_id);
            dbus_if_send_status (context, request->policy_id, NGF_STATUS_SUCCESS);
            remove_request = TRUE;
            break;

        default:
            break;
    }

    if (remove_request) {
        N_INFO ("request (%d) >> removed", request->policy_id);

        context->request_list = g_list_remove (context->request_list, request);
        stop_request (request);
        request_free (request);
    }
}

guint
play_handler (Context *context, const char *request_name, NProplist *proplist)
{
    Definition *def     = NULL;
    Event      *event   = NULL;
    Request    *request = NULL;

    guint policy_id    = 0;
    guint play_timeout = 0;
    gint  resources    = 0;
    gint  play_mode    = 0;

    const char *event_name      = NULL;

    TIMESTAMP ("Request request received");

    /* Get the policy identifier */
    policy_id    = _properties_get_policy_id (proplist);
    if (policy_id == 0) {
        N_WARNING ("No policy.id defined for request %s", request_name);
        return 0;
    }

    /* First, look for the request definition that defines the events for long and
       short requests. If not found, then it is an unrecognized request. */

    if ((def = g_hash_table_lookup (context->definitions, request_name)) == NULL) {
        N_WARNING ("No request definition for request %s", request_name);
        dbus_if_send_status (context, policy_id, NGF_STATUS_FAILED);
        return 0;
    }

    /* Then, get the play mode from the passed properties. The play mode is either "long"
       or "short" and we have a corresponding events defined in the request definition.
       If no play mode then this is an invalid request. */

    if ((play_mode = _properties_get_play_mode (proplist)) == 0) {
        N_WARNING ("No play.mode property for request %s", request_name);
        dbus_if_send_status (context, policy_id, NGF_STATUS_FAILED);
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
        N_WARNING ("Failed to get request event %s for request %s", event_name, request_name);
        dbus_if_send_status (context, policy_id, NGF_STATUS_FAILED);
        return 0;
    }

    /* Get allowed resources and play mode and timeout
       for our request. */

    play_timeout = _properties_get_play_timeout (proplist);
    resources    = _properties_get_resources (proplist);

    if (policy_id == 0 || resources == 0) {
        N_WARNING ("No resources defined for request %s", request_name);
        dbus_if_send_status (context, policy_id, NGF_STATUS_FAILED);
        return 0;
    }

    N_INFO ("request_name=%s, event_name=%s, policy_id=%d, play_timeout=%d, resources=0x%X, play_mode=%d (%s))",
        request_name, event_name, policy_id, play_timeout, resources, play_mode, play_mode == REQUEST_PLAY_MODE_LONG ? "LONG" : "SHORT");
    N_DEBUG ("Properties:");
    n_proplist_dump (proplist);

    TIMESTAMP ("Request parsing completed");

    /* Create a new request based on the event and feed the properties
       to it. */

    if ((request = request_new (context, event)) == NULL) {
        N_WARNING ("request (%d) >> failed create request", policy_id);
        dbus_if_send_status (context, policy_id, NGF_STATUS_FAILED);
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
        request_set_custom_sound (request, n_proplist_get_string (proplist, "audio"));

    context->request_list = g_list_append (context->request_list, request);
    if (!play_request (request)) {
        N_WARNING ("request (%d) >> failed to start", request->policy_id);
        dbus_if_send_status (context, policy_id, NGF_STATUS_FAILED);
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
        if (policy_id == 0 || (request->policy_id == policy_id)) {
            N_INFO ("request (%d) >> stop received", policy_id);

            context->request_list = g_list_remove (context->request_list, request);

            stop_request (request);
            request_free (request);

            dbus_if_send_status (context, policy_id, 0);
            break;
        }
    }
}

void
pause_handler (Context *context, guint policy_id, gboolean pause)
{
    Request *request = NULL;
    GList *iter = NULL;

    if (context->request_list == NULL)
        return;

    for (iter = g_list_first (context->request_list); iter; iter = g_list_next (context->request_list)) {
        request = (Request*) iter->data;
        if (policy_id == 0 || (request->policy_id == policy_id)) {
            NGF_LOG_INFO ("request (%d) >> %s received", policy_id,
                pause ? "pause" : "resume");

            if (pause)
                pause_request (request);
            else
                resume_request (request);
            break;
        }
    }
}

