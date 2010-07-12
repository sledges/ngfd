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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include <pulse/context.h>
#include <pulse/stream.h>

#include <sndfile.h>

#include "log.h"
#include "audio-pulse.h"

#define PULSE_BACKEND_NAME "NGF Pulse backend"
#define MAX_BUFFER_SIZE    65536

typedef struct _PulseStream PulseStream;

struct _PulseStream
{
    pa_stream         *stream;
    uint32_t           stream_index;
    int                fd;
    int                fd_error;
    SNDFILE           *sf;
    pa_operation      *drain_op;
    unsigned char      buffer[MAX_BUFFER_SIZE];
    AudioInterface    *iface;
};

static gboolean _pulse_initialize (AudioInterface *iface, PulseContext *context);
static void     _pulse_shutdown   (AudioInterface *iface);
static gboolean _pulse_prepare    (AudioInterface *iface, AudioStream *stream);
static gboolean _pulse_play       (AudioInterface *iface, AudioStream *stream);
static void     _pulse_stop       (AudioInterface *iface, AudioStream *stream);

static void
_stream_drain_cb (pa_stream *s, int success, void *userdata)
{
    AudioStream *stream       = (AudioStream*) userdata;
    PulseStream    *pulse_stream = (PulseStream*) stream->data;

    _pulse_stop (pulse_stream->iface, stream);

    if (stream->callback)
        stream->callback (stream,AUDIO_STREAM_STATE_COMPLETED, stream->userdata);

    pa_operation_unref (pulse_stream->drain_op);
    pulse_stream->drain_op = NULL;
}


static void
_stream_write_cb (pa_stream *s,
                  size_t     bytes,
                  void      *userdata)
{
    AudioStream *stream       = (AudioStream*) userdata;
    PulseStream    *pulse_stream = (PulseStream*) stream->data;

    size_t bytes_left = bytes;
    size_t bytes_read;

    while (1) {
	    if (bytes_left <= 0)
	        break;

	    bytes_read = sf_read_raw (pulse_stream->sf, &pulse_stream->buffer[0], bytes_left >= MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : bytes_left);
	    if (bytes_read == 0)
	        goto stream_drain;

    if (pa_stream_write (s, &pulse_stream->buffer[0], bytes_read, NULL, 0, PA_SEEK_RELATIVE) < 0)
        goto stream_drain;

	    bytes_left -= bytes_read;
    }

    return;

stream_drain:
    pa_stream_set_write_callback (s, NULL, NULL);
    pulse_stream->drain_op = pa_stream_drain (s, _stream_drain_cb, stream);

    return;
}

static void
_stream_state_cb (pa_stream *s,
                  void      *userdata)
{
    AudioStream *stream       = (AudioStream*) userdata;
    PulseStream    *pulse_stream = (PulseStream*) stream->data;

    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    switch (pa_stream_get_state (s)) {
        case PA_STREAM_READY:
	        pulse_stream->stream_index = pa_stream_get_index (s);

            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_PREPARED, stream->userdata);

            break;

        case PA_STREAM_FAILED:
            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_FAILED, stream->userdata);

	        break;

        case PA_STREAM_TERMINATED:
            if (stream->callback)
                stream->callback (stream, AUDIO_STREAM_STATE_FAILED, stream->userdata);

	        break;

        default:
            break;
    }
}

static pa_sample_format_t
_get_pa_sample_format (int format)
{
    int subformat = format & SF_FORMAT_SUBMASK;

    switch (subformat) {
        case SF_FORMAT_PCM_U8:
	        return PA_SAMPLE_U8;

        case SF_FORMAT_ALAW:
	        return PA_SAMPLE_ALAW;

        case SF_FORMAT_ULAW:
	        return PA_SAMPLE_ULAW;

        case SF_FORMAT_PCM_16:
	        return PA_SAMPLE_S16LE;
            /* return PA_SAMPLE_S16BE; */

        case SF_FORMAT_FLOAT:
	        return PA_SAMPLE_FLOAT32LE;
            /* return PA_SAMPLE_FLOAT32BE; */

        case SF_FORMAT_PCM_32:
	        return PA_SAMPLE_S32LE;
            /* return PA_SAMPLE_S32BE; */

        default:
            break;
    }

	return PA_SAMPLE_INVALID;
}

static gboolean
_pulse_initialize (AudioInterface *iface,
                   PulseContext   *context)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    iface->data = (gpointer) context;
    return TRUE;
}

static void
_pulse_shutdown (AudioInterface *iface)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    (void) iface;
}

static gboolean
_pulse_play (AudioInterface *iface,
                AudioStream    *stream)
{
    pa_operation *o = NULL;
    PulseStream  *pulse_stream = stream->data;

    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    (void) iface;

    if (pulse_stream->stream) {
        o = pa_stream_cork (pulse_stream->stream, 0, NULL, NULL);
        if (o)
            pa_operation_unref (o);
    }

    if (stream->callback)
        stream->callback (stream, AUDIO_STREAM_STATE_STARTED, stream->userdata);

    return TRUE;
}

static gboolean
_pulse_prepare (AudioInterface *iface,
             AudioStream    *stream)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    PulseStream     *pulse_stream = NULL;
    pa_sample_spec   ss;
    SF_INFO          sf_info;
    pa_proplist     *proplist;

    PulseContext *context = (PulseContext*) iface->data;
    pa_context      *c       = pulse_context_get_context (context);

    if (stream->source == NULL)
        return FALSE;

    if ((pulse_stream = g_new0 (PulseStream, 1)) == NULL)
        return FALSE;

    /* Save the reference to our private data. */
    pulse_stream->iface = iface;
    stream->data        = (gpointer) pulse_stream;

	if ((pulse_stream->fd = open (stream->source, O_RDONLY)) <0) {
        pulse_stream->fd_error = errno;
	    g_warning ("Unable to open file descriptor '%s': %s (%d)", stream->source, strerror (pulse_stream->fd_error), pulse_stream->fd_error);
        goto failed;
	}

    if ((pulse_stream->sf = sf_open_fd (pulse_stream->fd, SFM_READ, &sf_info, FALSE)) == NULL)
        goto failed;

    ss.format   = _get_pa_sample_format (sf_info.format);
	ss.channels = sf_info.channels;
	ss.rate     = sf_info.samplerate;

    if (ss.format == PA_SAMPLE_INVALID)
        goto failed;

    proplist = pa_proplist_copy (stream->properties);
    pa_proplist_sets (proplist, PA_PROP_MEDIA_FILENAME, stream->source);

    pulse_stream->stream = pa_stream_new_with_proplist (c, PULSE_BACKEND_NAME, &ss, NULL, proplist);
    if (pulse_stream->stream == NULL) {
        pa_proplist_free (proplist);
        goto failed;
    }

    pa_proplist_free (proplist);

    pa_stream_set_state_callback (pulse_stream->stream, _stream_state_cb, stream);
    pa_stream_set_write_callback (pulse_stream->stream, _stream_write_cb, stream);

    if (pa_stream_connect_playback (pulse_stream->stream, NULL, NULL, PA_STREAM_START_CORKED, NULL, NULL) < 0)
        goto failed;

    return TRUE;

failed:
    _pulse_stop (iface, stream);
    return FALSE;
}

static void
_pulse_stop (AudioInterface *iface,
             AudioStream    *stream)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    PulseStream *s = (PulseStream*) stream->data;

    if (s->stream) {
        pa_stream_set_state_callback (s->stream, NULL, NULL);
        pa_stream_set_write_callback (s->stream, NULL, NULL);
        pa_stream_disconnect (s->stream);
        pa_stream_unref (s->stream);
        s->stream = NULL;
    }

    if (s->sf) {
        sf_close (s->sf);
        s->sf = NULL;
    }

    if (s->fd > -1) {
        close (s->fd);
        s->fd = -1;
    }
}

AudioInterface*
audio_pulse_create ()
{
    static AudioInterface iface = {
        .initialize = _pulse_initialize,
        .shutdown   = _pulse_shutdown,
        .prepare    = _pulse_prepare,
        .play       = _pulse_play,
        .stop       = _pulse_stop
    };

    return (AudioInterface*) &iface;
}
