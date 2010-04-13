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

#include <glib.h>
#include <gst/gst.h>
#include <gio/gio.h>

#include "ngf-log.h"
#include "ngf-audio-gstreamer.h"

static gboolean _gst_initialize (NgfAudioInterface *iface, NgfPulseContext *context);
static void     _gst_shutdown   (NgfAudioInterface *iface);
static gboolean _gst_prepare    (NgfAudioInterface *iface, NgfAudioStream *stream);
static gboolean _gst_play       (NgfAudioInterface *iface, NgfAudioStream *stream);
static void     _gst_stop       (NgfAudioInterface *iface, NgfAudioStream *stream);

static gboolean
_bus_cb (GstBus     *bus,
         GstMessage *msg,
         gpointer    userdata)
{
    NgfAudioStream *stream     = (NgfAudioStream*) userdata;
    GstElement     *element    = (GstElement*) stream->data;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR: {
            gst_element_set_state (element, GST_STATE_NULL);

            if (stream->callback)
                stream->callback (stream, NGF_AUDIO_STREAM_STATE_FAILED, stream->userdata);

            _gst_stop (stream->iface, stream);
            return FALSE;
        }

        case GST_MESSAGE_STATE_CHANGED: {

            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != element)
                break;

            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);

            if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_PLAYING) {
                if (stream->callback)
                    stream->callback (stream, NGF_AUDIO_STREAM_STATE_STARTED, stream->userdata);
            }
            break;
        }

        case GST_MESSAGE_EOS: {
            if (GST_ELEMENT (GST_MESSAGE_SRC (msg)) != element)
                break;

            if (stream->callback)
                stream->callback (stream, NGF_AUDIO_STREAM_STATE_COMPLETED, stream->userdata);

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

	if (!(element = gst_element_factory_make (name, NULL))) {
		LOG_WARNING ("Preloading element %s failed", name);
	}
	
	g_object_unref(element);
}

static gboolean
_gst_initialize (NgfAudioInterface * iface, NgfPulseContext * context)
{
	LOG_DEBUG ("%s >> entering", __FUNCTION__);

	(void) iface;
	(void) context;

	gst_init_check (NULL, NULL, NULL);

	_gst_element_preload ("aacparse");
	_gst_element_preload ("nokiaaacdec");
	_gst_element_preload ("id3demux");
	_gst_element_preload ("uridecodebin");
	_gst_element_preload ("mp3parse");
	_gst_element_preload ("nokiamp3dec");
	_gst_element_preload ("wavparse");
	_gst_element_preload ("oggdemux");
	_gst_element_preload ("tremor");
	_gst_element_preload ("filesrc");
	_gst_element_preload ("decodebin2");
	_gst_element_preload ("pulsesink");

	return TRUE;
}

static void
_gst_shutdown (NgfAudioInterface *iface)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    (void) iface;
}

static gboolean
_gst_prepare (NgfAudioInterface *iface,
              NgfAudioStream    *stream)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    GstElement *element   = NULL;
    GstElement *source    = NULL;
    GstElement *decodebin = NULL;
    GstElement *sink      = NULL;
    GstBus     *bus       = NULL;

    element = gst_pipeline_new (NULL);

    source    = gst_element_factory_make ("filesrc", NULL);
    decodebin = gst_element_factory_make ("decodebin2", NULL);
    sink      = gst_element_factory_make ("pulsesink", NULL);

    if (element == NULL || source == NULL || decodebin == NULL || sink == NULL)
        goto failed;

    gst_bin_add_many (GST_BIN (element), source, decodebin, sink, NULL);

    if (!gst_element_link (source, decodebin))
        goto failed_link;

    g_object_set (G_OBJECT (source), "location", stream->source, NULL);
    g_object_set (G_OBJECT (sink), "proplist", pa_proplist_copy (stream->properties), NULL);

    g_signal_connect (G_OBJECT (decodebin), "new-decoded-pad", G_CALLBACK (_new_decoded_pad_cb), sink);

    bus = gst_element_get_bus (element);
    gst_bus_add_watch (bus, _bus_cb, stream);
    gst_object_unref (bus);

    stream->data = (gpointer) element;
    gst_element_set_state (element, GST_STATE_PAUSED);

    return TRUE;

failed:
    if (element)   gst_object_unref (element);
    if (source)    gst_object_unref (source);
    if (decodebin) gst_object_unref (decodebin);
    if (sink)      gst_object_unref (sink);
    return FALSE;

failed_link:
    if (element)   gst_object_unref (element);
    return FALSE;
}

static gboolean
_gst_play (NgfAudioInterface *iface,
           NgfAudioStream    *stream)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    GstElement *element = (GstElement*) stream->data;
    gst_element_set_state (element, GST_STATE_PLAYING);
    return TRUE;
}

static void
_gst_stop (NgfAudioInterface *iface,
           NgfAudioStream    *stream)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    GstElement *element = (GstElement*) stream->data;

    if (element) {
        gst_element_set_state (element, GST_STATE_NULL);
        gst_object_unref (element);
        stream->data = NULL;
    }
}

NgfAudioInterface*
ngf_audio_gstreamer_create ()
{
    static NgfAudioInterface iface = {
        .initialize = _gst_initialize,
        .shutdown   = _gst_shutdown,
        .prepare    = _gst_prepare,
        .play       = _gst_play,
        .stop       = _gst_stop
    };

    return (NgfAudioInterface*) &iface;
}
