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

#include "log.h"
#include "audio-gstreamer.h"

static gboolean _gst_initialize (AudioInterface *iface);
static void     _gst_shutdown   (AudioInterface *iface);
static gboolean _gst_prepare    (AudioInterface *iface, AudioStream *stream);
static gboolean _gst_play       (AudioInterface *iface, AudioStream *stream);
static void     _gst_stop       (AudioInterface *iface, AudioStream *stream);

static gboolean
_bus_cb (GstBus     *bus,
         GstMessage *msg,
         gpointer    userdata)
{
    AudioStream *stream     = (AudioStream*) userdata;
    GstElement     *element    = (GstElement*) stream->data;
    GstQuery *query = NULL;
    gboolean res = FALSE;
    gdouble volume = 0;
    gint64 duration = 0;
    GstController *controller = (GstController*) stream->data2;
    GstInterpolationControlSource *csource = NULL;
    gdouble val = 0;
    GValue vol = { 0, };


    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR: {
            gst_element_set_state (element, GST_STATE_NULL);

            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_FAILED, stream->userdata);

            _gst_stop (stream->iface, stream);
            return FALSE;
        }

        case GST_MESSAGE_STATE_CHANGED: {

            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != element)
                break;

            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);

            if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED) {
                if (stream->callback)
                    stream->callback (stream, AUDIO_STREAM_STATE_PREPARED, stream->userdata);
            }

            if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_PLAYING) {
                stream->num_repeat++;
                if (stream->callback)
                    stream->callback (stream, AUDIO_STREAM_STATE_STARTED, stream->userdata);
            }
            break;
        }

        case GST_MESSAGE_EOS: {
            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != element)
                break;

            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_COMPLETED, stream->userdata);

            if (stream->repeating) {
                if (stream->volume->type == VOLUME_TYPE_LINEAR) {
                    query = gst_query_new_duration (GST_FORMAT_TIME);
                    res = gst_element_query (element, query);
                    if (res) {
                        gst_query_parse_duration (query, NULL, &duration);
                        duration /= 1000000000;
                        duration *= stream->num_repeat;
                    } else {
                        NGF_LOG_WARNING ("Audio duration query failed");
                        gst_query_unref (query);
                        return TRUE;
                    }
                    gst_query_unref (query);

                    if (controller) {
                        if (duration >= stream->volume->linear[2]) {
                            gst_controller_set_disabled (controller, TRUE);
                        } else {
                            volume=(gdouble)duration*(((gdouble)stream->volume->linear[1]-(gdouble)stream->volume->linear[0])/(gdouble)stream->volume->linear[2]);
                            csource = gst_interpolation_control_source_new ();
                            gst_controller_set_control_source (controller, "volume", GST_CONTROL_SOURCE (csource));
                            gst_interpolation_control_source_set_interpolation_mode (csource, GST_INTERPOLATE_LINEAR);
                            g_value_init (&vol, G_TYPE_DOUBLE);

                            g_value_set_double (&vol, volume / 100.0);
                            gst_interpolation_control_source_set (csource, duration * GST_SECOND, &vol);

                            val = (gdouble)stream->volume->linear[1];
                            g_value_set_double (&vol, val / 100.0);
                            gst_interpolation_control_source_set (csource, (stream->volume->linear[2]-duration) * GST_SECOND, &vol);

                            g_object_unref (csource);
                        }
                    }
                }

                gst_element_seek_simple (element, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 0);

                return TRUE;
            } else {
                _gst_stop (stream->iface, stream);
                return FALSE;
            }
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
		NGF_LOG_WARNING ("Preloading element %s failed", name);
		return;
	}

	g_object_unref (element);
}

static gboolean
_gst_initialize (AudioInterface * iface)
{
	NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

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
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    (void) iface;
}

static gboolean
_gst_prepare (AudioInterface *iface,
              AudioStream    *stream)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    GstElement  *element   = NULL;
    GstElement  *source    = NULL;
    GstElement  *decodebin = NULL;
    GstElement  *volume    = NULL;
    GstElement  *sink      = NULL;
    GstBus      *bus       = NULL;
    pa_proplist *proplist  = NULL;
    GstController *controller = NULL;
    GstInterpolationControlSource *csource = NULL;
    gdouble val = 0;
    GValue vol = { 0, };

    if (!stream->source)
        return FALSE;

    element = gst_pipeline_new (NULL);

    source    = gst_element_factory_make ("filesrc", NULL);
    decodebin = gst_element_factory_make ("decodebin2", NULL);
    sink      = gst_element_factory_make ("pulsesink", NULL);

    if (element == NULL || source == NULL || decodebin == NULL || sink == NULL)
        goto failed;

    if (stream->volume && stream->volume->type == VOLUME_TYPE_LINEAR) {
        if ((volume = gst_element_factory_make ("volume", NULL)) == NULL)
            goto failed;

        if ((controller = gst_controller_new (G_OBJECT (volume), "volume", NULL)) == NULL)
            goto failed;
        csource = gst_interpolation_control_source_new ();
        gst_controller_set_control_source (controller, "volume", GST_CONTROL_SOURCE (csource));
        gst_interpolation_control_source_set_interpolation_mode (csource, GST_INTERPOLATE_LINEAR);

        g_value_init (&vol, G_TYPE_DOUBLE);

        val = (gdouble)stream->volume->linear[0];
        g_value_set_double (&vol, val / 100.0);
        gst_interpolation_control_source_set (csource, 0 * GST_SECOND, &vol);

        val = (gdouble)stream->volume->linear[1];
        g_value_set_double (&vol, val / 100.0);
        gst_interpolation_control_source_set (csource, stream->volume->linear[2] * GST_SECOND, &vol);

        g_object_unref (csource);
        gst_bin_add_many (GST_BIN (element), source, decodebin, volume, sink, NULL);
        if (!gst_element_link (volume, sink))
            goto failed_link;
        g_signal_connect (G_OBJECT (decodebin), "new-decoded-pad", G_CALLBACK (_new_decoded_pad_cb), volume);
    } else {
        gst_bin_add_many (GST_BIN (element), source, decodebin, sink, NULL);
        g_signal_connect (G_OBJECT (decodebin), "new-decoded-pad", G_CALLBACK (_new_decoded_pad_cb), sink);
    }

    if (!gst_element_link (source, decodebin))
        goto failed_link;

    g_object_set (G_OBJECT (source), "location", stream->source, NULL);

    proplist = pa_proplist_copy (stream->properties);
    pa_proplist_sets (proplist, PA_PROP_MEDIA_FILENAME, stream->source);

    if (g_object_class_find_property (G_OBJECT_GET_CLASS (sink), "proplist") != NULL) {
        NGF_LOG_DEBUG ("Setting property list for pulsesink.");
        g_object_set (G_OBJECT (sink), "proplist", proplist, NULL);
    }
    else {
        NGF_LOG_DEBUG ("No 'proplist' property on pulsesink, ignoring property list.");
        pa_proplist_free (proplist);
    }

    if (stream->buffer_time > 0)
        g_object_set (G_OBJECT (sink), "buffer-time", stream->buffer_time, NULL);

    if (stream->latency_time > 0)
        g_object_set (G_OBJECT (sink), "latency-time", stream->latency_time, NULL);

    bus = gst_element_get_bus (element);
    gst_bus_add_watch (bus, _bus_cb, stream);
    gst_object_unref (bus);

    stream->data = (gpointer) element;
    stream->data2 = (gpointer) controller;
    stream->num_repeat = 0;
    gst_element_set_state (element, GST_STATE_PAUSED);

    return TRUE;

failed:
    if (element)   gst_object_unref (element);
    if (source)    gst_object_unref (source);
    if (decodebin) gst_object_unref (decodebin);
    if (volume)     gst_object_unref (volume);
    if (sink)      gst_object_unref (sink);
    if (controller) gst_object_unref (controller);
    return FALSE;

failed_link:
    if (element)   gst_object_unref (element);
    return FALSE;
}

static gboolean
_gst_play (AudioInterface *iface,
           AudioStream    *stream)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    GstElement *element = (GstElement*) stream->data;
    gst_element_set_state (element, GST_STATE_PLAYING);
    return TRUE;
}

static void
_gst_stop (AudioInterface *iface,
           AudioStream    *stream)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    GstElement *element = (GstElement*) stream->data;
    GstController *controller = (GstController*) stream->data2;

    if (element) {
        gst_element_set_state (element, GST_STATE_NULL);
        gst_object_unref (element);
        stream->data = NULL;
    }

    if (controller) {
        g_object_unref (controller);
        stream->data2 = NULL;
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
        .stop       = _gst_stop
    };

    return (AudioInterface*) &iface;
}
