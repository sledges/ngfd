#include "core-player.h"

#define N_KEY_PLAY_DATA "core.sync_data"
#define LOG_CAT         "core: "

typedef struct _NPlayData NPlayData;

struct _NPlayData
{
    NCore    *core;
    NRequest *request;
    guint     play_source_id;   /* source id for play */
    guint     stop_source_id;   /* source id for stop */
    GList    *all_sinks;        /* all sinks available for the request */
    GList    *sinks_preparing;  /* sinks not yet synchronized and still preparing */
    GList    *sinks_playing;    /* sinks currently playing */
};

static int      n_core_sink_priority_cmp        (gconstpointer in_a, gconstpointer in_b);
static gboolean n_core_sink_synchronize_done_cb (gpointer userdata);
static gboolean n_core_request_done_cb          (gpointer userdata);



static int
n_core_sink_priority_cmp (gconstpointer in_a, gconstpointer in_b)
{
    NSinkInterface *a = (NSinkInterface*) in_a;
    NSinkInterface *b = (NSinkInterface*) in_b;

    if (a->priority > b->priority)
        return -1;
    else if (b->priority > a->priority)
        return 1;

    return 0;
}

static gboolean
n_core_sink_synchronize_done_cb (gpointer userdata)
{
    NPlayData      *play_data = (NPlayData*) userdata;
    NCore          *core      = play_data->core;
    NRequest       *request   = play_data->request;
    GList          *iter      = NULL;
    NSinkInterface *sink      = NULL;

    /* all sinks have been synchronized for the request. call play for every
       sink. */

    play_data->play_source_id = 0;
    for (iter = g_list_first (play_data->all_sinks); iter; iter = g_list_next (iter)) {
        sink = (NSinkInterface*) iter->data;

        if (!sink->funcs.play (sink, request)) {
            N_WARNING (LOG_CAT "sink '%s' failed play request '%s'",
                sink->name, request->name);

            n_core_fail_sink (core, sink, request);
            break;
        }
    }

    return FALSE;
}

static gboolean
n_core_request_done_cb (gpointer userdata)
{
    NPlayData      *play_data = (NPlayData*) userdata;
    NCore          *core      = play_data->core;
    NRequest       *request   = play_data->request;
    GList          *iter      = NULL;
    NSinkInterface *sink      = NULL;

    /* all sinks have been either completed or the request failed. we will run
       a stop on each sink and then clear out the request. */

    play_data->stop_source_id = 0;
    core->requests = g_list_remove (core->requests, request);

    N_DEBUG (LOG_CAT "stopping all sinks for request '%s'", request->name);
    for (iter = g_list_first (play_data->all_sinks); iter; iter = g_list_next (iter)) {
        sink = (NSinkInterface*) iter->data;
        sink->funcs.stop (sink, request);
    }

    g_list_free (play_data->sinks_playing);
    g_list_free (play_data->sinks_preparing);
    g_list_free (play_data->all_sinks);
    g_slice_free (NPlayData, play_data);

    /* send the reply to the input interface, if any. */

    if (request->input_iface->funcs.send_reply) {
        request->input_iface->funcs.send_reply (request->input_iface,
            request, 0);
    }

    /* free the actual request */
    N_DEBUG (LOG_CAT "request '%s' done", request->name);
    n_request_free (request);

    return FALSE;
}

int
n_core_play_request (NCore *core, NRequest *request)
{
    g_assert (core != NULL);
    g_assert (request != NULL);

    NPlayData       *play_data = NULL;
    NProplist       *props     = NULL;
    NSinkInterface **sink_iter = NULL;
    NSinkInterface  *sink      = NULL;
    GList           *all_sinks = NULL;
    GList           *iter      = NULL;

    /* evaluate the request and context to resolve the correct event for
       this specific request. if no event, then there is no default event
       defined and we are done here. */

    request->event = n_core_evaluate_request (core, request);
    if (!request->event) {
        N_WARNING (LOG_CAT "unable to resolve event for request '%s'",
            request->name);
        return FALSE;
    }

    N_DEBUG (LOG_CAT "request '%s' resolved to event '%s'", request->name,
        request->event->name);

    /* copy the event properties and merge request properties to it */

    props = n_proplist_copy (request->event->properties);
    n_proplist_merge (props, request->properties);
    n_proplist_free (request->properties);
    request->properties = props;

    /* query if the sinks can handle the request */

    for (sink_iter = core->sinks; *sink_iter; ++sink_iter) {
        if ((*sink_iter)->funcs.can_handle && !(*sink_iter)->funcs.can_handle (*sink_iter, request))
            continue;

        all_sinks = g_list_append (all_sinks, *sink_iter);
    }

    if (!all_sinks) {
        N_WARNING (LOG_CAT "no sinks that can handle the request '%s'",
            request->name);
        return FALSE;
    }

    /* sort the sinks based on their priority. priority is set automatically for
       each sink if "core.sink_order" key is set. */

    all_sinks = g_list_sort (all_sinks, n_core_sink_priority_cmp);

    /* create the play and synchronization data */

    play_data = g_slice_new0 (NPlayData);
    play_data->core            = core;
    play_data->request         = request;
    play_data->all_sinks       = all_sinks;
    play_data->sinks_preparing = g_list_copy (all_sinks);
    play_data->sinks_playing   = g_list_copy (all_sinks);

    /* store the sync data for later reference */

    n_request_store_data (request, N_KEY_PLAY_DATA, play_data);

    /* prepare all sinks that can handle the event. if there is no preparation
       function defined within the sink, then it is synchronized immediately. */

    core->requests = g_list_append (core->requests, request);

    for (iter = g_list_first (all_sinks); iter; iter = g_list_next (iter)) {
        sink = (NSinkInterface*) iter->data;

        if (!sink->funcs.prepare) {
            N_DEBUG (LOG_CAT "sink has no prepare, synchronizing immediately");
            n_core_synchronize_sink (core, sink, request);
            continue;
        }

        if (!sink->funcs.prepare (sink, request)) {
            N_WARNING (LOG_CAT "sink '%s' failed to prepare request '%s'",
                sink->name, request->name);

            n_core_fail_sink (core, sink, request);
            break;
        }
    }

    return TRUE;
}

int
n_core_pause_request (NCore *core, NRequest *request)
{
    g_assert (core != NULL);
    g_assert (request != NULL);

    NPlayData      *play_data = NULL;
    GList          *iter      = NULL;
    NSinkInterface *sink      = NULL;

    play_data = (NPlayData*) n_request_get_data (request, N_KEY_PLAY_DATA);
    g_assert (play_data != NULL);

    for (iter = g_list_first (play_data->all_sinks); iter; iter = g_list_next (iter)) {
        sink = (NSinkInterface*) iter->data;

        if (sink->funcs.pause && !sink->funcs.pause (sink, request)) {
            N_WARNING (LOG_CAT "sink '%s' failed to pause request '%s'",
                sink->name, request->name);
        }
    }

    return TRUE;
}

void
n_core_stop_request (NCore *core, NRequest *request)
{
    g_assert (core != NULL);
    g_assert (request != NULL);

    NPlayData *play_data = NULL;

    play_data = (NPlayData*) n_request_get_data (request, N_KEY_PLAY_DATA);
    g_assert (play_data != NULL);

    if (play_data->stop_source_id > 0) {
        N_DEBUG (LOG_CAT "already stopping request '%s'", request->name);
        return;
    }

    if (play_data->play_source_id > 0) {
        g_source_remove (play_data->play_source_id);
        play_data->play_source_id = 0;
    }

    play_data->stop_source_id = g_idle_add (n_core_request_done_cb, play_data);
}

void
n_core_synchronize_sink (NCore *core, NSinkInterface *sink, NRequest *request)
{
    g_assert (core != NULL);
    g_assert (sink != NULL);
    g_assert (request != NULL);

    NPlayData *play_data = NULL;

    N_DEBUG (LOG_CAT "sink '%s' ready to play request '%s'",
        sink->name, request->name);

    play_data = (NPlayData*) n_request_get_data (request, N_KEY_PLAY_DATA);
    g_assert (play_data != NULL);

    play_data->sinks_preparing = g_list_remove (play_data->sinks_preparing, sink);
    if (!play_data->sinks_preparing) {
        N_DEBUG (LOG_CAT "all sinks have been synchronized");
        play_data->play_source_id = g_idle_add (n_core_sink_synchronize_done_cb,
            play_data);
    }
}

void
n_core_complete_sink (NCore *core, NSinkInterface *sink, NRequest *request)
{
    g_assert (core != NULL);
    g_assert (sink != NULL);
    g_assert (request != NULL);

    NPlayData *play_data = NULL;

    play_data = (NPlayData*) n_request_get_data (request, N_KEY_PLAY_DATA);
    if (!play_data->sinks_playing)
        return;

    N_DEBUG (LOG_CAT "sink '%s' completed request '%s'",
        sink->name, request->name);

    play_data->sinks_playing = g_list_remove (play_data->sinks_playing, sink);
    if (!play_data->sinks_playing) {
        N_DEBUG (LOG_CAT "all sinks have been completed");
        play_data->stop_source_id = g_idle_add (n_core_request_done_cb,
            play_data);
    }
}

void
n_core_fail_sink (NCore *core, NSinkInterface *sink, NRequest *request)
{
    g_assert (core != NULL);
    g_assert (sink != NULL);
    g_assert (request != NULL);

    NPlayData *play_data = NULL;

    play_data = (NPlayData*) n_request_get_data (request, N_KEY_PLAY_DATA);
    N_WARNING (LOG_CAT "sink '%s' failed request '%s'",
        sink->name, request->name);

    if (play_data->stop_source_id > 0)
        return;

    /* sink failed, so request failed */
    play_data = (NPlayData*) n_request_get_data (request, N_KEY_PLAY_DATA);
    play_data->stop_source_id = g_idle_add (n_core_request_done_cb, play_data);
}
