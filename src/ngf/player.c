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

#include <string.h>

#include "log.h"
#include "event.h"
#include "resources.h"
#include "tone-generator.h"
#include "led.h"
#include "backlight.h"
#include "player.h"

#define AUDIO_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_AUDIO)

#define VIBRATION_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_VIBRATION)

#define LEDS_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_LEDS)

#define BACKLIGHT_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_BACKLIGHT)

static gboolean          max_timeout_cb                   (gpointer userdata);
static void              setup_timeout                    (Request *request);
static void              remove_timeout                   (Request *request);

static void              set_stream_event_id              (AudioStream *stream, const char *event_id);
static void              set_stream_role_from_volume      (AudioStream *stream, Volume *volume);

static const gchar*      get_uncompressed_tone            (Context *context, const char *tone);
static SoundPath*        resolve_sound_path               (Request *request, gboolean advance);

static gchar*            build_vibration_filename         (const char *path, const char *source);
static VibrationPattern* resolve_custom_pattern           (Request *request, SoundPath *sound_path);
static VibrationPattern* resolve_vibration_pattern        (Request *request, gboolean advance);

static void              synchronize_resources            (Request *request);
static gboolean          restart_next_stream              (Request *request);
static void              resynchronize_resources          (Request *request);
static void              stream_state_cb                  (AudioStream *stream, AudioStreamState state, gpointer userdata);

static gboolean          prepare_stream                   (Request *request, SoundPath *sound_path);
static gboolean          play_stream                      (Request *request);
static void              stop_stream                      (Request *request);
static void              pause_stream                     (Request *request);
static void              resume_stream                    (Request *request);

static void              vibration_completed_cb           (Vibrator *vibrator, gpointer userdata);
static gboolean          play_vibration                   (Request *request, VibrationPattern *pattern);
static void              stop_vibration                   (Request *request);
static void              pause_vibration                  (Request *request);
static void              resume_vibration                 (Request *request);

static gboolean          playback_path_tone_generator     (Request *request);
static gboolean          playback_path_synchronize        (Request *request);
static gboolean          playback_path_vibration          (Request *request);
static gboolean          playback_path_leds_and_backlight (Request *request);
static gboolean          playback_path_no_sound           (Request *request);



static gboolean
max_timeout_cb (gpointer userdata)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Request *request = (Request*) userdata;

    /* it is enough to emit the completed event to the
       caller. */

    request->max_timeout_id = 0;
    if (request->callback)
        request->callback (request, REQUEST_STATE_COMPLETED, request->userdata);

    return FALSE;
}

static void
setup_timeout (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Event   *event   = request->event;
    guint    timeout = 0;

    if (request->max_timeout_id > 0)
        g_source_remove (request->max_timeout_id);

    timeout = (event->max_timeout > 0 && (event->max_timeout < request->play_timeout)) ? event->max_timeout : request->play_timeout;
    if (timeout <= 0)
        return;

    NGF_LOG_DEBUG ("%s >> set to %d milliseconds", __FUNCTION__, timeout);
    request->max_timeout_id = g_timeout_add (timeout, max_timeout_cb, request);
}

static void
remove_timeout (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    if (request->max_timeout_id > 0) {
        g_source_remove (request->max_timeout_id);
        request->max_timeout_id = 0;
    }
}

static void
set_stream_event_id (AudioStream *stream, const char *event_id)
{
    GValue v = {0,{{0}}};

    if (!stream || (stream && !stream->properties))
        return;

    if (event_id) {
        NGF_LOG_DEBUG ("%s >> set stream event id to %s", __FUNCTION__, event_id);
        g_value_init (&v, G_TYPE_STRING);
        g_value_set_string (&v, event_id);
        gst_structure_set_value (stream->properties, "event.id", &v);
        g_value_unset (&v);
    }
}

static void
set_stream_role_from_volume (AudioStream *stream, Volume *volume)
{
    const char *value = NULL;
    GValue v = {0,{{0}}};

    if (!stream || (stream && !stream->properties))
        return;

    value = (volume && volume->role) ? volume->role : "x-maemo";
    NGF_LOG_DEBUG ("%s >> set stream role to %s", __FUNCTION__, value);

    g_value_init (&v, G_TYPE_STRING);
    g_value_set_string (&v, value);
    gst_structure_set_value (stream->properties, "module-stream-restore.id", &v);
    g_value_unset (&v);
}

static const gchar*
get_uncompressed_tone (Context *context, const char *tone)
{
    const char *uncompressed = NULL;

    if (context == NULL || tone == NULL)
        return NULL;

    uncompressed = tone_mapper_get_tone (context, tone);
    if (uncompressed) {
        NGF_LOG_DEBUG ("%s >> resolved uncompressed tone: %s", __FUNCTION__, uncompressed);
        return uncompressed;
    }

    return NULL;
}

static SoundPath*
resolve_sound_path (Request *request, gboolean advance)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    SoundPath *sound_path = NULL;

    if (advance && request->custom_sound) {
        sound_path_free (request->custom_sound);
        request->custom_sound = NULL;

        if (request->sound_iterator)
            sound_path = (SoundPath*) request->sound_iterator->data;
    }

    else if (advance && !request->custom_sound) {
        if ((request->sound_iterator = g_list_next (request->sound_iterator)) == NULL)
            return NULL;

        sound_path = (SoundPath*) request->sound_iterator->data;
    }

    else if (!advance && request->custom_sound) {
        sound_path = request->custom_sound;
    }

    else if (!advance && !request->custom_sound) {
        if (request->sound_iterator)
            sound_path = (SoundPath*) request->sound_iterator->data;
    }

    if (!sound_path)
        return NULL;

    NGF_LOG_DEBUG ("%s >> (type=%d, filename=%s, key=%s, profile=%s)", __FUNCTION__,
        sound_path->type, sound_path->filename, sound_path->key, sound_path->profile);

    return sound_path;
}

static gchar*
build_vibration_filename (const char *path, const char *source)
{
    gchar *separator = NULL;
    gchar *output    = NULL;
    gchar *basename  = NULL;
    gchar *result    = NULL;

    if (!source)
        return NULL;

    if (!path) {
        basename = g_strdup (source);
        if ((separator = g_strrstr (basename, ".")) == NULL) {
            g_free (basename);
            return NULL;
        }

        *separator = '\0';
        result = g_strdup_printf ("%s.ivt", basename);

        g_free (output);
        g_free (basename);
    }
    else {
        basename = g_path_get_basename (source);
        if ((separator = g_strrstr (basename, ".")) == NULL) {
            g_free (basename);
            return NULL;
        }

        *separator = '\0';
        output = g_strdup_printf ("%s.ivt", basename);
        result = g_build_filename (path, output, NULL);

        g_free (output);
        g_free (basename);
    }

    return result;
}

static VibrationPattern*
resolve_custom_pattern (Request *request, SoundPath *sound_path)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context          *context  = request->context;
    VibrationPattern *pattern  = NULL;
    gpointer          data     = NULL;
    gchar            *filename = NULL;

    if (!sound_path || (sound_path && !sound_path->filename))
        return NULL;

    if ((filename = build_vibration_filename (context->patterns_path, sound_path->filename)) == NULL)
        return NULL;

    if ((data = vibrator_load (filename)) == NULL) {
        g_free (filename);
        return NULL;
    }

    pattern = vibration_pattern_new ();
    pattern->type     = VIBRATION_PATTERN_TYPE_FILENAME;
    pattern->filename = filename;
    pattern->data     = data;

    NGF_LOG_DEBUG ("%s >> custom pattern (type=%d, filename=%s, pattern=%d)", __FUNCTION__,
        pattern->type, pattern->filename, pattern->pattern);

    return pattern;
}

static VibrationPattern*
resolve_vibration_pattern (Request *request, gboolean advance)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    VibrationPattern *pattern = NULL;

    if (advance)
        request->vibration_iterator = g_list_next (request->vibration_iterator);

    if (request->vibration_iterator == NULL)
        return NULL;

    pattern = (VibrationPattern*) request->vibration_iterator->data;

    NGF_LOG_DEBUG ("%s >> pattern (type=%d, filename=%s, pattern=%d)", __FUNCTION__,
        pattern->type, pattern->filename, pattern->pattern);

    return pattern;
}

static void
synchronize_resources (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    if (!request->synchronize_done) {
        request->synchronize_done = TRUE;

        setup_timeout                    (request);
        play_stream                      (request);
        playback_path_leds_and_backlight (request);
        playback_path_vibration          (request);

        return;
    }

    if (request->active_pattern && request->custom_pattern)
        (void) play_vibration (request, request->active_pattern);
    else
        playback_path_vibration          (request);
}

static gboolean
restart_next_stream (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    SoundPath *sound_path = NULL;
    stop_stream (request);

    if ((sound_path = resolve_sound_path (request, TRUE)) == NULL)
        return FALSE;

    return prepare_stream (request, sound_path);
}

static void
resynchronize_resources (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    /* resynchronize is called upon when the stream is rewinded back
       to the start and put to paused state. we want to restart the vibration
       with the sound, if we have a custom pattern. otherwise, we just put
       the stream back to playing. */

    if (request->custom_pattern && request->active_pattern) {
        stop_vibration (request);
        play_vibration (request, request->active_pattern);
    }

    play_stream (request);
}

static void
stream_state_cb (AudioStream *stream, AudioStreamState state, gpointer userdata)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    (void) stream;

    Request *request        = (Request*) userdata;
    guint    callback_state = REQUEST_STATE_NONE;

    switch (state) {
        case AUDIO_STREAM_STATE_PREPARED:
            NGF_LOG_DEBUG ("%s >> prepared", __FUNCTION__);
            synchronize_resources (request);
            break;

        case AUDIO_STREAM_STATE_STARTED:
            NGF_LOG_DEBUG ("%s >> started", __FUNCTION__);
            callback_state = REQUEST_STATE_STARTED;
            break;

        case AUDIO_STREAM_STATE_FAILED:
            NGF_LOG_DEBUG ("%s >> failed", __FUNCTION__);
            if (!restart_next_stream (request))
                callback_state = REQUEST_STATE_FAILED;
            break;

        case AUDIO_STREAM_STATE_REWIND:
            NGF_LOG_DEBUG ("%s >> repeat", __FUNCTION__);
            resynchronize_resources (request);
            break;

        case AUDIO_STREAM_STATE_COMPLETED:
            NGF_LOG_DEBUG ("%s >> completed", __FUNCTION__);
            callback_state = REQUEST_STATE_COMPLETED;
            break;

        default:
            break;
    }

    if (callback_state != REQUEST_STATE_NONE && request->callback)
        request->callback (request, callback_state, request->userdata);
}

static gboolean
prepare_stream (Request *request, SoundPath *sound_path)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context     *context       = request->context;
    Event       *event         = request->event;
    const gchar *uncompressed  = NULL;
    gchar       *stream_source = NULL;
    guint        stream_type   = 0;
    AudioStream *stream        = NULL;

    /* tone mapper provides an uncompressed file for us to play
       if available. If volume controller is to be used, force gstreamer
       playback.
    */

    uncompressed = get_uncompressed_tone (context, sound_path->filename);
    if (request->event->volume->type != VOLUME_TYPE_LINEAR && uncompressed) {
        stream_source = g_strdup (uncompressed);
        stream_type   = AUDIO_STREAM_UNCOMPRESSED;
    }
    else {
        stream_source = g_strdup (sound_path->filename);
        stream_type   = AUDIO_STREAM_NONE;
    }

    /* create audio stream and prepare it for playback */

    stream = audio_create_stream (context->audio, stream_type);

    stream->source         = g_strdup (stream_source);
    stream->buffer_time    = context->audio_buffer_time;
    stream->latency_time   = context->audio_latency_time;
    stream->properties     = gst_structure_empty_new ("props");
    stream->callback       = stream_state_cb;
    stream->userdata       = request;
    stream->volume         = event->volume;
    stream->num_repeats    = event->num_repeats;
    stream->repeating      = event->repeat;

    request->stream       = stream;
    request->active_sound = sound_path;

    set_stream_event_id         (stream, event->event_id);
    set_stream_role_from_volume (stream, event->volume);

    if (!audio_prepare (context->audio, request->stream)) {
        audio_destroy_stream (context->audio, request->stream);
        request->stream       = NULL;
        request->active_sound = NULL;
        return FALSE;
    }

    return TRUE;
}

static gboolean
play_stream (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (!audio_play (context->audio, request->stream)) {
        audio_destroy_stream (context->audio, request->stream);
        request->stream = NULL;
        return FALSE;
    }

    return TRUE;
}

static void
stop_stream (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (request->stream) {
        audio_stop (context->audio, request->stream);
        audio_destroy_stream (context->audio, request->stream);
        request->stream = NULL;
    }
}

static void
pause_stream (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (request->stream) {
        audio_pause (context->audio, request->stream);
    }
}

static void
resume_stream (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (request->stream) {
        audio_play (context->audio, request->stream);
    }
}

static void
vibration_completed_cb (Vibrator *vibrator, gpointer userdata)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    (void) vibrator;

    Request *request = (Request*) userdata;

    /* do a resource update or complete the request, when the vibration
       pattern is finite and no audio stream is played. */

    if (request->stream == NULL) {
        if (request->callback)
            request->callback (request, REQUEST_STATE_COMPLETED, request->userdata);
    }
}

static gboolean
play_vibration (Request *request, VibrationPattern *pattern)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (!pattern)
        return FALSE;

    if ((request->vibration_id = vibrator_start (context->vibrator, pattern->data, pattern->pattern, vibration_completed_cb, request)) > 0)
        return TRUE;

    return FALSE;
}

static void
stop_vibration (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (request->vibration_id > 0) {
        vibrator_stop (context->vibrator, request->vibration_id);
        request->vibration_id = 0;
    }
}

static void
pause_vibration (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (request->vibration_id > 0) {
        vibrator_pause (context->vibrator, request->vibration_id);
    }
}

static void
resume_vibration (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;

    if (request->vibration_id > 0) {
        vibrator_resume (context->vibrator, request->vibration_id);
    }
}

static gboolean
playback_path_tone_generator (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    tone_generator_start (context->system_bus, event->tone_generator_pattern);
    request->tone_generator_active = TRUE;

    return TRUE;
}

static gboolean
playback_path_synchronize (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    SoundPath *sound_path = NULL;

    /* resolve the sound path we try first. if we don't get anything
       then fallback to the no sound path. */

    if ((sound_path = resolve_sound_path (request, FALSE)) == NULL)
        return playback_path_no_sound (request);

    /* prepare the stream for playback. */
    return prepare_stream (request, sound_path);
}

static gboolean
playback_path_vibration (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context          *context = request->context;
    Event            *event   = request->event;
    VibrationPattern *pattern = NULL;
    gboolean          advance = FALSE;

    if (!context->vibration_enabled || !event->vibration_enabled || !VIBRATION_RESOURCE_ENABLED (request))
        return FALSE;

    /* query for the custom sound based on the filename. if that fails, fallback to the
       event's pattern. */

    if (event->lookup_pattern && request->active_sound && (request->custom_pattern = resolve_custom_pattern (request, request->active_sound)) != NULL) {
        if (play_vibration (request, request->custom_pattern)) {
            request->active_pattern = request->custom_pattern;
            return TRUE;
        }
    }

    while ((pattern = resolve_vibration_pattern (request, advance)) != NULL) {
        if (play_vibration (request, pattern)) {
            request->active_pattern = pattern;
            break;
        }

        advance = TRUE;
    }

    return request->active_pattern != NULL ? TRUE : FALSE;
}

static gboolean
playback_path_leds_and_backlight (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    if (event->leds_enabled && LEDS_RESOURCE_ENABLED (request) && event->led_pattern) {
        led_activate_pattern (context->system_bus, event->led_pattern);
        request->leds_active = TRUE;
    }

    if (event->backlight_enabled && BACKLIGHT_RESOURCE_ENABLED (request)) {
        backlight_display_on (context->system_bus);
        request->backlight_active = TRUE;
    }

    return TRUE;
}

static gboolean
playback_path_no_sound (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    setup_timeout                    (request);
    playback_path_vibration          (request);
    playback_path_leds_and_backlight (request);

    return TRUE;
}

int
play_request (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    /* reset the sound and vibration iterators to the first entries in the
       lists. */

    request->sound_iterator     = g_list_first (event->sounds);
    request->vibration_iterator = g_list_first (event->patterns);

    /* tone generator path does not active any other resource, except it
       triggers the knocking sound. */

    if (event->tone_generator_enabled)
        return playback_path_tone_generator (request);

    /* sync the vibration, led and backlight to the audio if
       audio resource is available and audio is enabled if we are
       not in silent mode. */

    if (event->audio_enabled && AUDIO_RESOURCE_ENABLED (request)) {
        if ((context->silent_mode && event->silent_enabled) || !context->silent_mode)
            return playback_path_synchronize (request);
    }

    /* no audio resource, start the vibration, led and backlight
       immediately. */

    return playback_path_no_sound (request);
}

int
stop_request (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    remove_timeout      (request);
    stop_vibration      (request);
    stop_stream         (request);

    if (request->leds_active) {
        led_deactivate_pattern (context->system_bus, event->led_pattern);
        request->leds_active = FALSE;
    }

    if (request->custom_pattern) {
        vibration_pattern_free (request->custom_pattern);
        request->custom_pattern = NULL;
    }

    return TRUE;
}

void
pause_request (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    pause_stream    (request);
    pause_vibration (request);
}

void
resume_request (Request *request)
{
    NGF_LOG_ENTER ("%s >> entering", __FUNCTION__);

    resume_stream    (request);
    resume_vibration (request);
}

