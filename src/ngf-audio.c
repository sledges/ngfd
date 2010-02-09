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

#include "ngf-audio.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include <pulse/pulseaudio.h>
#include <pulse/context.h>
#include <pulse/stream.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/introspect.h>

#include <sndfile.h>

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
    pa_stream           *stream;
    guint               stream_id;
    uint32_t            stream_index;

    pa_proplist         *proplist;

    int                 fd;
    int                 fd_error;
    SNDFILE             *sf;

    unsigned char       buffer[MAX_BUFFER_SIZE];

    pa_operation        *drain_op;

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
static gboolean pulseaudio_create_stream (NgfAudio *self, AudioStream *stream);
static void     pulseaudio_destroy_stream (NgfAudio *self, AudioStream *stream);
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

    g_free (stream->filename);
    stream->filename = NULL;

    g_free (stream);
}

static void
stream_drain_cb (pa_stream *s, int success, void *userdata)
{
    AudioStream *stream = (AudioStream*) userdata;

    pulseaudio_destroy_stream (stream->audio, stream);

    if (stream->callback)
        stream->callback (stream->audio, stream->stream_id, NGF_STREAM_COMPLETED, stream->userdata);

    pa_operation_unref (stream->drain_op);
    stream->drain_op = NULL;

    audio_stream_free (stream);
}

static void
stream_write_cb (pa_stream *s, size_t bytes, void *userdata)
{
    AudioStream *stream = (AudioStream*) userdata;

    pa_operation *o = NULL;
    size_t bytes_left = bytes;
    size_t bytes_read;

    while (1) {
	    if (bytes_left <= 0)
	        break;

	    bytes_read = sf_read_raw (stream->sf, &stream->buffer[0], bytes_left >= MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : bytes_left);
	    if (bytes_read == 0)
	        goto stream_drain;

    	if (pa_stream_write (s, &stream->buffer[0], bytes_read, NULL, 0, PA_SEEK_RELATIVE) < 0)
    	    goto stream_drain;

	    bytes_left -= bytes_read;
    }

    return;

stream_drain:
    pa_stream_set_write_callback (s, NULL, NULL);
    stream->drain_op = pa_stream_drain (s, stream_drain_cb, stream);

    return;
}

static void
stream_state_cb (pa_stream *s, void *userdata)
{
    AudioStream *stream = (AudioStream*) userdata;

    switch (pa_stream_get_state (s)) {
        case PA_STREAM_UNCONNECTED:
        case PA_STREAM_CREATING:
	        break;

        case PA_STREAM_READY:
	        stream->stream_index = pa_stream_get_index (s);

            if (stream->callback)
                stream->callback (stream->audio, stream->stream_id, NGF_STREAM_STARTED, stream->userdata);

        	break;

        case PA_STREAM_FAILED:
	        pulseaudio_destroy_stream (stream->audio, stream);

            if (stream->callback)
                stream->callback (stream->audio, stream->stream_id, NGF_STREAM_FAILED, stream->userdata);

            audio_stream_free (stream);
	        break;

        case PA_STREAM_TERMINATED:
	        pulseaudio_destroy_stream (stream->audio, stream);

            if (stream->callback)
                stream->callback (stream->audio, stream->stream_id, NGF_STREAM_TERMINATED, stream->userdata);

            audio_stream_free (stream);
	        break;

        default:
            break;
    }
}

static void
start_queued_streams (NgfAudio *self)
{
    AudioStream *stream = NULL;
    GList *iter = NULL;

    for (iter = g_list_first (self->stream_queue); iter; iter = g_list_next (iter)) {
        stream = (AudioStream*) iter->data;

        if (!pulseaudio_create_stream (self, stream)) {
            g_free (stream->filename);
            g_free (stream);
        }
        else {
            self->active_streams = g_list_append (self->active_streams, stream);
        }
    }

    g_list_free (self->stream_queue);
    self->stream_queue = NULL;
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

            if (self->stream_requested) {
                start_queued_streams (self);
                self->stream_requested = FALSE;
            }

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

static pa_sample_format_t
sf_to_pa_sample_format (int format)
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
pulseaudio_create_stream (NgfAudio *self, AudioStream *stream)
{
    SF_INFO sf_info;
    pa_sample_spec ss;

    if (stream->filename == NULL)
        goto failed;

	if ((stream->fd = open (stream->filename, O_RDONLY)) <0) {
        stream->fd_error = errno;
	    g_warning ("Unable to open file descriptor '%s': %s (%d)", stream->filename, strerror (stream->fd_error), stream->fd_error);
        goto failed;
	}

    if ((stream->sf = sf_open_fd (stream->fd, SFM_READ, &sf_info, FALSE)) == NULL)
        goto failed;

    ss.format = sf_to_pa_sample_format (sf_info.format);
	ss.channels = sf_info.channels;
	ss.rate = sf_info.samplerate;

    if (ss.format == PA_SAMPLE_INVALID)
        goto failed;

    stream->stream = pa_stream_new_with_proplist (self->context, APPLICATION_NAME, &ss, NULL, stream->proplist);
    if (stream->stream == NULL) {
        goto failed;
    }

    pa_stream_set_state_callback (stream->stream, stream_state_cb, stream);
    pa_stream_set_write_callback (stream->stream, stream_write_cb, stream);

    if (pa_stream_connect_playback (stream->stream, NULL, NULL, 0, NULL, NULL) < 0)
        goto failed;

    self->active_streams = g_list_append (self->active_streams, stream);
    return TRUE;

failed:
    pulseaudio_destroy_stream (self, stream);
    return FALSE;

}

static void
pulseaudio_destroy_stream (NgfAudio *self, AudioStream *stream)
{
    if (stream->stream) {
    	pa_stream_set_state_callback (stream->stream, NULL, NULL);
    	pa_stream_set_write_callback (stream->stream, NULL, NULL);
    	pa_stream_disconnect (stream->stream);
    	pa_stream_unref (stream->stream);
    	stream->stream = NULL;
    }

    if (stream->sf) {
    	sf_close (stream->sf);
    	stream->sf = NULL;
    }

    if (stream->fd > -1) {
    	close (stream->fd);
    	stream->fd = -1;
    }

    self->active_streams = g_list_remove (self->active_streams, stream);
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
#if 0
    pa_ext_stream_restore2_info stream_restore_info[1];

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

    stream_restore_info[0].name = role;
    stream_restore_info[0].channel_map.channels = 1;
    stream_restore_info[0].channel_map.map[0] = PA_CHANNEL_POSITION_MONO;
    stream_restore_info[0].volume = vol;
    stream_restore_info[0].device = NULL;
    stream_restore_info[0].mute = FALSE;
    stream_restore_info[0].volume_is_absolute = TRUE;

    o = pa_ext_stream_restore2_write (self->context, PA_UPDATE_REPLACE, stream_restore_info, 1, TRUE, NULL, NULL);

    if (o != NULL)
        pa_operation_unref (o);
#endif
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
        return 0;

    stream->stream_id = ++self->stream_count;
    stream->filename = g_strdup (filename);
    stream->proplist = p != NULL ? pa_proplist_copy (p) : NULL;
    stream->callback = callback;
    stream->userdata = userdata;
    stream->audio = self;

    if (pa_context_get_state (self->context) != PA_CONTEXT_READY) {
        self->stream_queue = g_list_append (self->stream_queue, stream);
        self->stream_requested = TRUE;
        return stream->stream_id;
    }

    /* Context is ready, we can start the stream immediately. */

    if (!pulseaudio_create_stream (self, stream)) {
        audio_stream_free (stream);
        return 0;
    }

    return stream->stream_id;
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
            if (stream->drain_op) {
                g_print ("DRAIN OP (stream_id=%d)\n", stream_id);
                pa_operation_cancel (stream->drain_op);
                pa_operation_unref (stream->drain_op);
            }

            pulseaudio_destroy_stream (self, stream);

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
