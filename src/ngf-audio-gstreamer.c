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
#include "ngf-audio.h"

#include <glib.h>
#include <gst/gst.h>
#include <gio/gio.h>

#include <pulse/pulseaudio.h>
#include <pulse/context.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/introspect.h>

#define APPLICATION_NAME    "ngf-audio-backend"
#define PACKAGE_VERSION     "0.1"
#define MAX_BUFFER_SIZE     65536

typedef struct _AudioSample
{
    uint32_t    index;
    int         lazy;
    gchar       *filename;
} AudioSample;

typedef struct _AudioStream
{
    gchar               *filename;
    GstElement          *element;
    guint               stream_id;

    pa_proplist         *proplist;

    NgfAudio            *audio;
    NgfStreamCallback   callback;
    gpointer            userdata;
} AudioStream;

struct _NgfAudio
{
    pa_glib_mainloop    *mainloop;
    pa_context          *context;

    GList               *active_streams;
    GList               *stream_queue;
    guint               stream_count;
    gboolean            stream_requested;

    gboolean            sample_cache_loaded;
    GHashTable          *sample_cache;

    NgfAudioCallback    callback;
    gpointer            userdata;
};

static gboolean pulseaudio_initialize (NgfAudio *self);
static void     pulseaudio_shutdown (NgfAudio *self);
static void     pulseaudio_get_cached_samples (NgfAudio *self);



static void
audio_sample_free (AudioSample *sample)
{
    g_free (sample->filename);
    g_free (sample);
}

static void
audio_stream_free (AudioStream *stream)
{
    if (stream == NULL)
        return;

    if (stream->proplist) {
        pa_proplist_free (stream->proplist);
        stream->proplist = NULL;
    }

    if (stream->element) {
        gst_object_unref (stream->element);
        stream->element = NULL;
    }

    g_free (stream->filename);
    stream->filename = NULL;

    g_free (stream);
}

static void
context_state_cb (pa_context *c, void *userdata)
{
    NgfAudio *self = (NgfAudio*) userdata;

    switch (pa_context_get_state (c)) {
        case PA_CONTEXT_CONNECTING:
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            /* Query a list of samples */
            pulseaudio_get_cached_samples (self);

            if (self->callback)
                self->callback (self, NGF_AUDIO_READY, self->userdata);

            break;

        case PA_CONTEXT_FAILED:
            if (self->callback)
                self->callback (self, NGF_AUDIO_FAILED, self->userdata);

            break;

        case PA_CONTEXT_TERMINATED:
            if (self->callback)
                self->callback (self, NGF_AUDIO_TERMINATED, self->userdata);

            break;

        default:
            break;
    }
}

static gboolean
pulseaudio_initialize (NgfAudio *self)
{
    pa_proplist *proplist = NULL;
    pa_mainloop_api *api = NULL;

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
    pa_context_set_state_callback (self->context, context_state_cb, self);

    if (pa_context_connect (self->context, NULL, /* PA_CONTEXT_NOFAIL | */ PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
    	return FALSE;
    }

    return TRUE;
}

static void
pulseaudio_shutdown (NgfAudio *self)
{
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

static void
sample_info_cb (pa_context *c, const pa_sample_info *i, int eol, void *userdata)
{
    NgfAudio *self = (NgfAudio*) userdata;

    if (eol) {

        /* Trigger a callback to notify client that we have the sample
           list. */

        self->sample_cache_loaded = TRUE;

        if (self->callback)
            self->callback (self, NGF_AUDIO_SAMPLE_LIST, self->userdata);

        return;
    }

    AudioSample *sample = NULL;

    sample = g_new0 (AudioSample, 1);
    sample->index    = i->index;
    sample->lazy     = i->lazy;
    sample->filename = g_strdup (i->filename);

    g_hash_table_insert (self->sample_cache, g_strdup (i->name), sample);
}

static void
pulseaudio_get_cached_samples (NgfAudio *self)
{
    pa_operation *o = NULL;

    if (self->sample_cache_loaded)
        return;

    o = pa_context_get_sample_info_list (self->context, sample_info_cb, self);
    if (o)
        pa_operation_unref (o);
}

NgfAudio*
ngf_audio_create ()
{
    NgfAudio *self = NULL;

    if ((self = g_new0 (NgfAudio, 1)) == NULL)
        goto failed;

    if ((self->sample_cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) audio_sample_free)) == NULL)
        goto failed;

    if (!pulseaudio_initialize (self))
        goto failed;

    gst_init_check (NULL, NULL, NULL);

    return self;

failed:
    ngf_audio_destroy (self);
    return NULL;
}

void
ngf_audio_destroy (NgfAudio *self)
{
    if (self == NULL)
        return;

    pulseaudio_shutdown (self);

    if (self->sample_cache) {
        g_hash_table_destroy (self->sample_cache);
        self->sample_cache = NULL;
    }

    g_free (self);
}

void
ngf_audio_set_callback (NgfAudio *self, NgfAudioCallback callback, gpointer userdata)
{
    if (self == NULL || callback == NULL)
        return;

    self->callback = callback;
    self->userdata = userdata;
}

void
ngf_audio_set_volume (NgfAudio *self, const char *role, gint volume)
{
    pa_ext_stream_restore2_info *stream_restore_info[1], info;

    pa_cvolume vol;
    gdouble v = 0.0;
    pa_operation *o = NULL;

    if (self->context == NULL)
        return;

    if (pa_context_get_state (self->context) != PA_CONTEXT_READY)
        return;

    if (volume < 0)
        return;

    volume = volume > 100 ? 100 : volume;
    v = (gdouble) volume / 100.0;

    pa_cvolume_set (&vol, 1, v * PA_VOLUME_NORM);

    info.name = role;
    info.channel_map.channels = 1;
    info.channel_map.map[0] = PA_CHANNEL_POSITION_MONO;
    info.volume = vol;
    info.device = NULL;
    info.mute = FALSE;
    info.volume_is_absolute = TRUE;

    stream_restore_info[0] = &info;

    o = pa_ext_stream_restore2_write (self->context, PA_UPDATE_REPLACE,
        (const pa_ext_stream_restore2_info *const *) stream_restore_info, 1, TRUE, NULL, NULL);
    if (o != NULL)
        pa_operation_unref (o);
}

static gchar*
_get_uri (const char *filename)
{
    GFile *file = NULL;
    gchar *uri = NULL;

    if ((file = g_file_new_for_path (filename)) == NULL)
        return NULL;

    uri = g_file_get_uri (file);
    g_object_unref (file);
    return uri;
}

static gboolean
_bus_cb (GstBus *bus, GstMessage *msg, gpointer userdata)
{
    AudioStream *stream = (AudioStream*) userdata;
    NgfAudio *self = stream->audio;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR: {
            gst_element_set_state (stream->element, GST_STATE_NULL);

            self->active_streams = g_list_remove (self->active_streams, stream);
            if (stream->callback)
                stream->callback (stream->audio, stream->stream_id, NGF_STREAM_FAILED, stream->userdata);

            audio_stream_free (stream);
            return FALSE;
        }

        case GST_MESSAGE_STATE_CHANGED: {

            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != stream->element)
                break;

            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);

            if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_PLAYING) {
                if (stream->callback)
                    stream->callback (stream->audio, stream->stream_id, NGF_STREAM_STARTED, stream->userdata);
            }
            break;
        }

        case GST_MESSAGE_EOS: {
            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != stream->element)
                break;

            self->active_streams = g_list_remove (self->active_streams, stream);
            if (stream->callback)
                stream->callback (stream->audio, stream->stream_id, NGF_STREAM_COMPLETED, stream->userdata);

            gst_element_set_state (stream->element, GST_STATE_NULL);
            audio_stream_free (stream);
            return FALSE;
        }

        default:
            break;
    }

    return TRUE;
}

static void
_new_decoded_pad_cb (GstElement *element, GstPad *pad, gboolean is_last, gpointer user_data)
{
    GstElement   *sink_element = (GstElement*) user_data;
    GstStructure *structure    = NULL;
    GstCaps      *caps         = NULL;
    GstPad       *sink_pad     = NULL;

    caps = gst_pad_get_caps (pad);
    if (gst_caps_is_empty (caps) || gst_caps_is_any (caps))
        return;

    structure = gst_caps_get_structure (caps, 0);
    if (g_str_has_prefix (gst_structure_get_name (structure), "audio")) {
        sink_pad = gst_element_get_pad (sink_element, "sink");
        if (!gst_pad_is_linked (sink_pad))
            gst_pad_link (pad, sink_pad);
        gst_object_unref (sink_pad);
    }

    gst_caps_unref (caps);
}

guint
ngf_audio_play_stream (NgfAudio *self, const char *filename, pa_proplist *p, NgfStreamCallback callback, gpointer userdata)
{
    AudioStream *stream = NULL;

    if (self == NULL || filename == NULL)
        return 0;

    if (self->context == NULL)
        return 0;

    if ((stream = g_new0 (AudioStream, 1)) == NULL)
        goto failed;

    stream->stream_id = ++self->stream_count;
    stream->filename  = g_strdup (filename);
    stream->proplist  = p != NULL ? pa_proplist_copy (p) : NULL;
    stream->callback  = callback;
    stream->userdata  = userdata;
    stream->audio     = self;

    stream->element = gst_pipeline_new (NULL);

    GstElement *source, *decodebin, *sink;
    GstBus *bus;

    source    = gst_element_factory_make ("filesrc", NULL);
    decodebin = gst_element_factory_make ("decodebin2", NULL);
    sink      = gst_element_factory_make ("pulsesink", NULL);

    if (stream->element == NULL || source == NULL || decodebin == NULL || sink == NULL)
        goto failed;

    gst_bin_add_many (GST_BIN (stream->element), source, decodebin, sink, NULL);

    if (!gst_element_link (source, decodebin))
        goto failed_link;

    g_object_set (G_OBJECT (source), "location", stream->filename, NULL);
    g_object_set (G_OBJECT (sink), "proplist", pa_proplist_copy (stream->proplist), NULL);

    g_signal_connect (G_OBJECT (decodebin), "new-decoded-pad", G_CALLBACK (_new_decoded_pad_cb), sink);

    bus = gst_element_get_bus (stream->element);
    gst_bus_add_watch (bus, _bus_cb, stream);
    gst_object_unref (bus);

    if (gst_element_set_state (stream->element, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
        goto failed_link;

    self->active_streams = g_list_append (self->active_streams, stream);

    return stream->stream_id;

failed:
    if (sink)
        gst_object_unref (sink);

    if (decodebin)
        gst_object_unref (decodebin);

    if (source)
        gst_object_unref (source);

    if (stream->element) {
        gst_object_unref (stream->element);
        stream->element = NULL;
    }

    _audio_stream_free (stream);
    return 0;

failed_link:
    gst_object_unref (stream->element);
    stream->element = NULL;
    _audio_stream_free (stream);
    return 0;
}

void
ngf_audio_stop_stream (NgfAudio *self, guint stream_id)
{
    AudioStream *stream = NULL;
    GList *iter = NULL;

    if (self == NULL || stream_id == 0)
        return;

    for (iter = g_list_first (self->active_streams); iter; iter = g_list_next (iter)) {
        stream = (AudioStream*) iter->data;
        if (stream->stream_id == stream_id) {

            if (stream->element)
                gst_element_set_state (stream->element, GST_STATE_NULL);

            if (stream->callback)
                stream->callback (self, stream->stream_id, NGF_STREAM_STOPPED, stream->userdata);

            audio_stream_free (stream);
            break;
        }
    }
}

GList*
ngf_audio_get_sample_list (NgfAudio *self)
{
    GList *sample_list = NULL;

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, self->sample_cache);
    while (g_hash_table_iter_next (&iter, &key, &value))
        sample_list = g_list_append (sample_list, g_strdup (key));

    return sample_list;
}

void
ngf_audio_free_sample_list (GList *sample_list)
{
    if (sample_list == NULL)
        return;

    GList *iter = NULL;
    for (iter = g_list_first (sample_list); iter; iter = g_list_next (iter))
        g_free ((gchar*) iter->data);

    g_list_free (sample_list);
}
