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

#include <glib.h>
#include <gst/gst.h>
#include <gst/controller/gstcontroller.h>
#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gio/gio.h>

#include <pulse/proplist.h>

#include "log.h"
#include "audio-gstreamer.h"

static gboolean structure_to_proplist_cb (GQuark field_id, const GValue *value, gpointer userdata);
static void     pipeline_rewind          (GstElement *pipeline, gboolean flush);
static void     set_stream_properties    (GstElement *sink, const GstStructure *properties);
static void     reset_linear_volume      (AudioStream *stream, gboolean query_position);

static gboolean _gst_initialize (AudioInterface *iface);
static void     _gst_shutdown   (AudioInterface *iface);
static gboolean _gst_prepare    (AudioInterface *iface, AudioStream *stream);
static gboolean _gst_play       (AudioInterface *iface, AudioStream *stream);
static void     _gst_stop       (AudioInterface *iface, AudioStream *stream);

static gboolean
structure_to_proplist_cb (GQuark field_id, const GValue *value, gpointer userdata)
{
    pa_proplist *proplist = (pa_proplist*) userdata;

    if (G_VALUE_HOLDS_STRING (value))
        pa_proplist_sets (proplist, g_quark_to_string (field_id), g_value_get_string (value));

    return TRUE;
}

static void
pipeline_rewind (GstElement *pipeline, gboolean flush)
{
    GstEvent *event;
    GstSeekFlags flags = GST_SEEK_FLAG_SEGMENT;

    if (flush)
        flags |= GST_SEEK_FLAG_FLUSH;

    event = gst_event_new_seek (1.0, GST_FORMAT_BYTES, flags, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_END, GST_CLOCK_TIME_NONE);
    if (!gst_element_send_event (pipeline, event)) {
        N_WARNING ("pipeline_rewind: failed to send event\n");
    }
}

static void
set_stream_properties (GstElement *sink, const GstStructure *properties)
{
    pa_proplist *proplist = NULL;

    if (!sink | !properties)
        return;

    if (g_object_class_find_property (G_OBJECT_GET_CLASS (sink), "stream-properties") != NULL) {
        g_object_set (G_OBJECT (sink), "stream-properties", properties, NULL);
    }

    else if (g_object_class_find_property (G_OBJECT_GET_CLASS (sink), "proplist") != NULL) {
        proplist = pa_proplist_new ();
        gst_structure_foreach (properties, structure_to_proplist_cb, proplist);
        g_object_set (G_OBJECT (sink), "proplist", proplist, NULL);

        /* no need ot unref proplist, ownership is taken by the sink */
    }
}

static void
reset_linear_volume (AudioStream *stream, gboolean query_position)
{
    Volume    *volume = stream->volume;
    GstFormat  fmt    = GST_FORMAT_TIME;
    GValue     v      = {0, {{0}}};

    gint64  timestamp;
    gdouble timeleft, current_volume;

    if (!volume || volume->type != VOLUME_TYPE_LINEAR)
        return;

    if (query_position) {
        if (!gst_element_query_position (stream->pipeline, &fmt, &timestamp)) {
            N_DEBUG ("%s >> unable to query stream position",
                __FUNCTION__);
            goto finish_controller;
        }

        if (!(GST_CLOCK_TIME_IS_VALID (timestamp) && fmt == GST_FORMAT_TIME)) {
            N_DEBUG ("%s >> queried position or format is not valid",
                __FUNCTION__);
            goto finish_controller;
        }

        stream->time_played += (gdouble) timestamp / GST_SECOND;
        timeleft = stream->volume->linear[2] - stream->time_played;

        g_object_get (G_OBJECT (stream->volume_element),
            "volume", &current_volume, NULL);
    }
    else {
        stream->time_played = 0.0;
        timeleft            = stream->volume->linear[2];
        current_volume      = stream->volume->linear[0] / 100.0;
    }

    if (timeleft > 0.0) {
        N_DEBUG ("%s >> query=%s, timeleft = %f, current_volume = %f\n",
            __FUNCTION__, query_position ? "TRUE" : "FALSE", timeleft,
            current_volume);

        gst_controller_set_disabled (stream->controller, TRUE);
        gst_interpolation_control_source_unset_all (stream->csource);

        g_value_init (&v, G_TYPE_DOUBLE);
        g_value_set_double (&v, current_volume);
        gst_interpolation_control_source_set (stream->csource,
            0 * GST_SECOND, &v);

        g_value_reset (&v);
        g_value_set_double (&v, (stream->volume->linear[1] / 100.0));
        gst_interpolation_control_source_set (stream->csource,
            timeleft * GST_SECOND, &v);

        g_value_unset (&v);
        gst_controller_set_disabled (stream->controller, FALSE);

        return;
    }

finish_controller:
    if (stream->controller) {
        N_DEBUG ("%s >> controller finished\n", __FUNCTION__);
        g_object_unref (stream->controller);
        stream->controller = NULL;
    }
}

static gboolean
_bus_cb (GstBus     *bus,
         GstMessage *msg,
         gpointer    userdata)
{
    (void) bus;

    AudioStream *stream = (AudioStream*) userdata;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR: {
            gst_element_set_state (stream->pipeline, GST_STATE_NULL);

            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_FAILED, stream->userdata);

            _gst_stop (stream->iface, stream);
            return FALSE;
        }

        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != stream->pipeline)
                break;

            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);

            if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED) {

                /* if the stream is not a repeating one, no need to send the initial flushing
                   segmented seek. instead we will wait for the eos to arrive. */

                if (stream->repeating) {
                    pipeline_rewind (stream->pipeline, TRUE);
                }

                if (stream->callback)
                    stream->callback (stream, AUDIO_STREAM_STATE_PREPARED, stream->userdata);
            }

            else if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_PLAYING) {
                if (stream->paused)
                    break;

                stream->current_repeat++;
                if (stream->callback)
                    stream->callback (stream, AUDIO_STREAM_STATE_STARTED, stream->userdata);
            }
            break;
        }

        case GST_MESSAGE_SEGMENT_DONE: {
            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != stream->pipeline)
                break;

            /* reset the linear volume controller values for the next iteration
               of the stream. if there is no linear volume, then just emit the
               rewind state change or completion. */

            reset_linear_volume (stream, TRUE);

            if (stream->num_repeats == 0 || (stream->current_repeat > stream->num_repeats)) {
                pipeline_rewind (stream->pipeline, FALSE);
                gst_element_set_state (stream->pipeline, GST_STATE_PAUSED);

                if (stream->callback)
                    stream->callback (stream, AUDIO_STREAM_STATE_REWIND,
                        stream->userdata);
            }
            else {
                if (stream->callback)
                    stream->callback (stream, AUDIO_STREAM_STATE_COMPLETED,
                        stream->userdata);

                _gst_stop (stream->iface, stream);
                return FALSE;
            }

            break;
        }

        case GST_MESSAGE_EOS: {
            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != stream->pipeline)
                break;

            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_COMPLETED, stream->userdata);

            _gst_stop (stream->iface, stream);
            return FALSE;
        }

        default:
            break;
    }

    return TRUE;
}

static void
_new_decoded_pad_cb (GstElement *element,
                     GstPad     *pad,
                     gboolean    is_last,
                     gpointer    userdata)
{
    (void) element;
    (void) is_last;

    GstElement   *sink_element = (GstElement*) userdata;
    GstStructure *structure    = NULL;
    GstCaps      *caps         = NULL;
    GstPad       *sink_pad     = NULL;

    caps = gst_pad_get_caps (pad);
    if (gst_caps_is_empty (caps) || gst_caps_is_any (caps)) {
        gst_caps_unref (caps);
        return;
    }

    structure = gst_caps_get_structure (caps, 0);
    if (g_str_has_prefix (gst_structure_get_name (structure), "audio")) {
        sink_pad = gst_element_get_pad (sink_element, "sink");
        if (!gst_pad_is_linked (sink_pad))
            gst_pad_link (pad, sink_pad);
        gst_object_unref (sink_pad);
    }

    gst_caps_unref (caps);
}

static void
_gst_element_preload (gchar * name)
{
	GstElement *element = NULL;

	if ((element = gst_element_factory_make (name, NULL)) == NULL) {
		N_WARNING ("Preloading element %s failed", name);
		return;
	}

	g_object_unref (element);
}

static gboolean
_gst_initialize (AudioInterface * iface)
{
	N_ENTER ("%s >> entering", __FUNCTION__);

	(void) iface;

    gst_init_check (NULL, NULL, NULL);
    gst_controller_init (NULL, NULL);

    _gst_element_preload ("aacparse");
    _gst_element_preload ("nokiaaacdec");
    _gst_element_preload ("id3demux");
    _gst_element_preload ("uridecodebin");
    _gst_element_preload ("mp3parse");
    _gst_element_preload ("nokiamp3dec");
    _gst_element_preload ("wavparse");
    _gst_element_preload ("oggdemux");
    _gst_element_preload ("ivorbisdec");
    _gst_element_preload ("filesrc");
    _gst_element_preload ("decodebin2");
    _gst_element_preload ("volume");
    _gst_element_preload ("pulsesink");

	return TRUE;
}

static void
_gst_shutdown (AudioInterface *iface)
{
    N_ENTER ("%s >> entering", __FUNCTION__);

    (void) iface;
}

static gboolean
_gst_prepare (AudioInterface *iface,
              AudioStream    *stream)
{
    N_ENTER ("%s >> entering", __FUNCTION__);

    (void) iface;

    GstElement   *source    = NULL;
    GstElement   *decodebin = NULL;
    GstElement   *sink      = NULL;
    GstBus       *bus       = NULL;
    GstStructure *props     = NULL;
    GValue        v         = {0,{{0}}};

    if (!stream->source)
        return FALSE;

    stream->pipeline = gst_pipeline_new (NULL);

    source    = gst_element_factory_make ("filesrc", NULL);
    decodebin = gst_element_factory_make ("decodebin2", NULL);
    sink      = gst_element_factory_make ("pulsesink", NULL);

    if (stream->pipeline == NULL || source == NULL || decodebin == NULL || sink == NULL)
        goto failed;

    if (stream->volume && stream->volume->type == VOLUME_TYPE_LINEAR) {
        if ((stream->volume_element = gst_element_factory_make ("volume", NULL)) == NULL)
            goto failed;

        if ((stream->controller = gst_controller_new (G_OBJECT (stream->volume_element), "volume", NULL)) == NULL)
            goto failed;

        stream->csource = gst_interpolation_control_source_new ();
        gst_controller_set_control_source (stream->controller, "volume", GST_CONTROL_SOURCE (stream->csource));
        gst_interpolation_control_source_set_interpolation_mode (stream->csource, GST_INTERPOLATE_LINEAR);

        reset_linear_volume (stream, FALSE);

        gst_bin_add_many (GST_BIN (stream->pipeline), source, decodebin,
            stream->volume_element, sink, NULL);

        if (!gst_element_link (stream->volume_element, sink))
            goto failed_link;

        g_signal_connect (G_OBJECT (decodebin), "new-decoded-pad", G_CALLBACK (_new_decoded_pad_cb), stream->volume_element);
    } else {
        gst_bin_add_many (GST_BIN (stream->pipeline), source, decodebin, sink, NULL);
        g_signal_connect (G_OBJECT (decodebin), "new-decoded-pad", G_CALLBACK (_new_decoded_pad_cb), sink);
    }

    if (!gst_element_link (source, decodebin))
        goto failed_link;

    g_object_set (G_OBJECT (source), "location", stream->source, NULL);

    /* copy property structure and append current filename */

    props = gst_structure_copy (stream->properties);

    g_value_init (&v, G_TYPE_STRING);
    g_value_set_string (&v, stream->source);
    gst_structure_set_value (props, "media.filename", &v);
    g_value_unset (&v);

    /* set the stream properties based on the given GstStructure */

    set_stream_properties (sink, props);
    gst_structure_free (props);

    if (stream->buffer_time > 0)
        g_object_set (G_OBJECT (sink), "buffer-time", stream->buffer_time, NULL);

    if (stream->latency_time > 0)
        g_object_set (G_OBJECT (sink), "latency-time", stream->latency_time, NULL);

    bus = gst_element_get_bus (stream->pipeline);
    gst_bus_add_watch (bus, _bus_cb, stream);
    gst_object_unref (bus);

    gst_element_set_state (stream->pipeline, GST_STATE_PAUSED);

    return TRUE;

failed:
    if (stream->pipeline)   gst_object_unref (stream->pipeline);
    if (source)    gst_object_unref (source);
    if (decodebin) gst_object_unref (decodebin);
    if (stream->volume_element)     gst_object_unref (stream->volume_element);
    if (sink)      gst_object_unref (sink);
    if (stream->controller) gst_object_unref (stream->controller);
    return FALSE;

failed_link:
    if (stream->pipeline)   gst_object_unref (stream->pipeline);
    return FALSE;
}

static gboolean
_gst_play (AudioInterface *iface,
           AudioStream    *stream)
{
    N_ENTER ("%s >> entering", __FUNCTION__);

    (void) iface;

    gst_element_set_state (stream->pipeline, GST_STATE_PLAYING);
    stream->paused = FALSE;
    return TRUE;
}

static void
_gst_pause (AudioInterface *iface,
            AudioStream    *stream)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    stream->paused = TRUE;
    gst_element_set_state (stream->pipeline, GST_STATE_PAUSED);
}

static void
_gst_stop (AudioInterface *iface,
           AudioStream    *stream)
{
    N_ENTER ("%s >> entering", __FUNCTION__);

    (void) iface;

    if (stream->pipeline) {
        gst_element_set_state (stream->pipeline, GST_STATE_NULL);
        gst_object_unref (stream->pipeline);
        stream->pipeline = NULL;
    }

    if (stream->controller) {
        gst_interpolation_control_source_unset_all (stream->csource);
        g_object_unref (stream->controller);
        stream->controller = NULL;
    }
}

AudioInterface*
audio_gstreamer_create ()
{
    static AudioInterface iface = {
        .initialize = _gst_initialize,
        .shutdown   = _gst_shutdown,
        .prepare    = _gst_prepare,
        .play       = _gst_play,
        .pause      = _gst_pause,
        .stop       = _gst_stop
    };

    return (AudioInterface*) &iface;
}
