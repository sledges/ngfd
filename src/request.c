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

#include <string.h>

#include "log.h"
#include "property.h"
#include "properties.h"
#include "tone-mapper.h"
#include "controller.h"
#include "request.h"

static inline void  _trigger_request_callback (Request *event, guint state);
static gboolean     _max_timeout_triggered_cb (gpointer userdata);

static void         _stream_state_cb (AudioStream *stream, AudioStreamState state, gpointer userdata);
static void         _interface_ready_cb (InterfaceType type, gboolean success, gpointer userdata);
static const char*  _get_mapped_tone (ToneMapper *mapper, const char *tone);

static gboolean     _tone_generator_start (Request *request);
static void         _tone_generator_stop (Request *request);

static void         _set_stream_volume  (Request *request);
static void         _clear_stream_volume (Request *request);

static gboolean     _audio_playback_start (Request *request, gboolean set_volume);
static void         _audio_playback_stop (Request *request);

static gboolean     _setup_vibrator (Request *request);
static gboolean     _poll_vibrator (gpointer userdata);
static gboolean     _setup_led (Request *request);
static gboolean     _setup_backlight (Request *request);
static void         _shutdown_vibrator (Request *request);
static void         _shutdown_led (Request *request);
static void         _shutdown_backlight (Request *request);



Request*
request_new (Context *context, EventPrototype *proto)
{
    Request *request = NULL;

    if (context == NULL || proto == NULL)
        return NULL;

    if ((request = g_new0 (Request, 1)) == NULL)
        return NULL;

    request->context     = context;
    request->proto       = proto;
    request->start_timer = g_timer_new ();

    return request;
}

void
request_free (Request *request)
{
    if (request == NULL)
        return;

    if (request->start_timer) {
        g_timer_destroy (request->start_timer);
        request->start_timer = NULL;
    }

    if (request->properties) {
        g_hash_table_destroy (request->properties);
        request->properties = NULL;
    }

    if (request->vibra_data) {
        g_free (request->vibra_data);
        request->vibra_data = NULL;
    }

    g_free (request);
}

void
request_set_callback (Request *request, RequestCallback callback, gpointer userdata)
{
    request->callback = callback;
    request->userdata = userdata;
}

static gboolean
_max_timeout_triggered_cb (gpointer userdata)
{
    Request *request = (Request*) userdata;

    request->max_length_timeout_id = 0;
    request_stop (request);

    _trigger_request_callback (request, REQUEST_STATE_COMPLETED);
    return FALSE;
}

/**
 * Get the uncompressed tone if there is one for the tone we wish
 * to play.
 *
 * @param mapper Instance of ToneMapper
 * @param tone Original tone
 * @return Full path to uncompressed tone
 */

static const char*
_get_mapped_tone (ToneMapper *mapper, const char *tone)
{
    const char *mapped_tone = NULL;

    if (mapper == NULL || tone == NULL)
        return NULL;

    mapped_tone = tone_mapper_get_tone (mapper, tone);
    if (mapped_tone) {
        LOG_DEBUG ("Tone (mapped): %s", mapped_tone);
        return mapped_tone;
    }

    return NULL;
}

/**
 * Trigger the user specified callback for the event. If no callback
 * specified, nothing is done.
 *
 * @param event Request
 * @param state guint
 * @post User provided callback is triggered with given state and userdata.
 */

static inline void
_trigger_request_callback (Request      *event,
                         guint  state)
{
    if (event->callback)
        event->callback (event, state, event->userdata);
}

/**
 * Interface ready callback is called when backend is ready to start
 * the event. Used to synchronize starting of backends at the same
 * time.
 *
 * @param type InterfaceType
 * @param success gboolean
 * @param userdata Userdata
 */

static void
_interface_ready_cb (InterfaceType type, gboolean success, gpointer userdata)
{
    Request *request = (Request*) userdata;
    const char *vibra = NULL;

    switch (type) {
        case INTERFACE_AUDIO:
            if (success) {
                LOG_DEBUG ("Audio backend ready");
                request->audio_ready = TRUE;
            } else {
                _stream_state_cb (request->audio_stream, AUDIO_STREAM_STATE_FAILED, request);
            }
            break;
        default:
            break;
    }

    if (properties_get_bool (request->properties, "audio_enabled")) {
        if (!request->audio_ready)
            return;
    }

    LOG_DEBUG ("All backends ready, starting event");
    if (properties_get_bool (request->properties, "audio_enabled")) {
        if (!audio_play (request->context->audio, request->audio_stream))
            audio_destroy_stream (request->context->audio, request->audio_stream);
    }

    if (request->resources & RESOURCE_VIBRATION && profile_is_vibra_enabled (request->context->profile)) {
        if (request->vibra_data) {
            request->vibra_id = vibrator_start (request->context->vibrator, NULL, request->vibra_data);
        } else {
            vibra = properties_get_string (request->properties, "vibra");
            if (vibra != NULL)
                request->vibra_id = vibrator_start (request->context->vibrator, vibra, NULL);
        }

        if (request->vibra_id && profile_is_silent (request->context->profile) &&
            !vibrator_is_repeating (request->context->vibrator, vibra)) {
            /* If we are in silent mode, set callback to monitor when pattern is complete, if pattern is non-repeating one */
            request->vibra_poll_id = g_timeout_add (VIBRA_POLL_TIMEOUT, _poll_vibrator, request);
        }
    }

    if (properties_get_bool (request->properties, "led_enabled"))
        _setup_led (request);

    if (properties_get_bool (request->properties, "backlight_enabled"))
        _setup_backlight (request);
}

/**
 * Stream state callback is triggered by the audio backend to inform us
 * of the streams state.
 *
 * @param stream AudioStream
 * @param state AudioStreamState
 * @param userdata Userdata
 */

static void
_stream_state_cb (AudioStream *stream, AudioStreamState state, gpointer userdata)
{
    Request *request = (Request*) userdata;

    guint callback_state = REQUEST_STATE_NONE;
    gboolean      restart_stream = FALSE;
    gboolean      set_volume     = FALSE;

    switch (state) {
        case AUDIO_STREAM_STATE_STARTED:
            callback_state = REQUEST_STATE_STARTED;
            break;

        case AUDIO_STREAM_STATE_FAILED: {

            /* If stream was a fallback stream and it failed, then trigger the
               callback and finish the event. */

            _audio_playback_stop (request);

            if (request->audio_use_fallback) {
                callback_state = REQUEST_STATE_FAILED;
                break;
            }

            request->audio_use_fallback = TRUE;
            set_volume                  = TRUE;
            restart_stream              = TRUE;

            break;
        }

        case AUDIO_STREAM_STATE_COMPLETED: {

            _audio_playback_stop (request);

            if (request->audio_repeat_enabled) {
                request->audio_repeat_count++;

                if (request->audio_max_repeats <= 0 || request->audio_repeat_count < request->audio_max_repeats) {
                    restart_stream = TRUE;
                    break;
                }
            }

            callback_state = REQUEST_STATE_COMPLETED;
            break;
        }

        default:
            break;
    }

    if (restart_stream)
        _audio_playback_start (request, set_volume);

    if (callback_state != REQUEST_STATE_NONE)
        _trigger_request_callback (request, callback_state);
}

static gboolean
_tone_generator_start (Request *request)
{
    if (properties_get_bool (request->properties, "audio_tone_generator_enabled")) {
        request->tone_generator_id = tone_generator_start (request->context->tonegen, properties_get_int (request->properties, "audio_tone_generator_pattern"));
        return TRUE;
    }

    return FALSE;
}

static void
_tone_generator_stop (Request *request)
{
    if (request->tone_generator_id > 0) {
        tone_generator_stop (request->context->tonegen, request->tone_generator_id);
        request->tone_generator_id = 0;
    }
}

static gboolean
_volume_controller_cb (Controller *controller,
                       guint          id,
                       guint          step_time,
                       guint          step_value,
                       gpointer       userdata)
{
    Request *request = (Request*) userdata;
    audio_set_volume (request->context->audio, properties_get_string (request->properties, "audio_stream_role"), step_value);
    return TRUE;
}

/**
 * Get audio filename from the the properties or if no such file, from
 * the profiles.
 *
 * @param request Request
 * @param filename_key Property key
 * @param profile_key Profile key
 * @returns Audio filename, do not free.
 */

static const char*
_get_audio_filename (Request   *request,
                     const char *filename_key,
                     const char *profile_key)
{
    const char *filename = NULL;
    const char *profile  = NULL;

    g_assert (request != NULL);

    if ((filename = properties_get_string (request->properties, filename_key)) != NULL)
        return filename;

    if ((profile = properties_get_string (request->properties, profile_key)) != NULL) {
        return profile_get_string_from_key (request->context->profile, profile);
    }

    return NULL;
}

/**
 * Get audio volume from the properties or from the profiles.
 *
 * @param request Request
 * @param volume_key Property key
 * @param profile_key Profile key
 * @returns Audio volume or -1 if no volume.
 */

static gint
_get_audio_volume (Request   *request,
                   const char *volume_key,
                   const char *profile_key)
{
    gint volume = 0;
    const char *profile = NULL;

    g_assert (request != NULL);

    if ((profile = properties_get_string (request->properties, profile_key)) != NULL) {
        return profile_get_int_from_key (request->context->profile, profile);
    }

    if ((volume = properties_get_int (request->properties, volume_key)) > -1)
        return volume;

    return -1;
}

static void
_set_stream_volume (Request *request)
{
    const char *pattern = NULL;
    gint        volume  = 0;

    if (request->audio_volume_set)
        return;

    pattern = properties_get_string (request->properties, "audio_volume_pattern");
    volume  = _get_audio_volume (request, "audio_volume_value", "audio_volume_profile");

    if (pattern != NULL) {
        request->audio_volume_controller = audio_get_controller (request->context->audio, pattern);
        request->audio_volume_id = controller_start (request->audio_volume_controller, _volume_controller_cb, request);
    }
    else if (volume >= 0) {
        audio_set_volume (request->context->audio, properties_get_string (request->properties, "audio_stream_role"), volume);
    }

    request->audio_volume_set = TRUE;
 }

static void
_clear_stream_volume (Request *request)
{
    if (request->audio_volume_id > 0) {
        controller_stop (request->audio_volume_controller, request->audio_volume_id);
        request->audio_volume_controller = NULL;
        request->audio_volume_id = 0;
    }

    request->audio_volume_set = FALSE;
}

static gboolean
_audio_playback_start (Request *request, gboolean set_volume)
{
    EventPrototype  *prototype   = request->proto;
    const char         *mapped      = NULL;
    const char         *source      = NULL;
    AudioStreamType  stream_type = 0;
    AudioStream     *stream      = NULL;

    if ((request->resources & RESOURCE_AUDIO) == 0)
        return FALSE;

    /* If we are in the silent mode and the prototype's audio_silent
       flag has not been set, nothing to do here. */

    if (profile_is_silent (request->context->profile) && !properties_get_bool (request->properties, "audio_silent"))
        return FALSE;

    if (request->audio_use_fallback)
        request->audio_filename = _get_audio_filename (request, "audio_fallback_filename", "audio_fallback_profile");

    source = request->audio_filename;

    /* set the stream volume */
    if (set_volume)
        _set_stream_volume (request);

    /* If we tried to get fallback and it did not exist, nothing to
       play here. */

    if (source == NULL && request->audio_use_fallback)
        return FALSE;

    /* There was no tone available, use the fallback and try again. */

    if (source == NULL && !request->audio_use_fallback) {
        request->audio_use_fallback = TRUE;
        return _audio_playback_start (request, FALSE);
    }

    /* Get the mapped (uncompressed) filename, if such thing exists and
       set the stream type to uncompressed. */

    mapped = _get_mapped_tone (request->context->tone_mapper, source);
    if (mapped != NULL) {
        source = mapped;
        stream_type = AUDIO_STREAM_UNCOMPRESSED;
    }

    /* Create a new audio stream and set it's properties. */

    stream = audio_create_stream (request->context->audio, stream_type);
    stream->source     = g_strdup (source);
    stream->properties = pa_proplist_copy (prototype->stream_properties);
    stream->iface_callback = _interface_ready_cb;
    stream->callback   = _stream_state_cb;
    stream->userdata   = request;

    /* Prepare the stream and start it's playback. */

    if (!audio_prepare (request->context->audio, stream)) {
        audio_destroy_stream (request->context->audio, stream);
        return FALSE;
    }

    request->audio_stream = stream;
    return TRUE;
}

static void
_audio_playback_stop (Request *request)
{
    _clear_stream_volume (request);

    if (request->audio_stream != NULL) {
        audio_stop (request->context->audio, request->audio_stream);
        audio_destroy_stream (request->context->audio, request->audio_stream);
        request->audio_stream = NULL;
    }
}

static gboolean
_poll_vibrator (gpointer userdata)
{
    Request *request = (Request*) userdata;

    if (request) {
        if (vibrator_is_completed (request->context->vibrator, request->vibra_id)) {
            _trigger_request_callback (request, REQUEST_STATE_COMPLETED);
            request->vibra_id = 0;
            request->vibra_poll_id = 0;
            return FALSE;
        } else
            return TRUE;
    }

    request->vibra_poll_id = 0;
    return FALSE;
}

static gchar*
_get_ivt_filename (const char *source)
{
    gchar *separator = NULL;
    gchar *output = NULL;
    size_t size;

    if (source == NULL)
        return NULL;

    separator = g_strrstr (source, ".");
    size = (separator != NULL) ? (size_t) (separator - source) : strlen (source);

    if (size == 0)
        return NULL;

    output = g_try_malloc0 (size + 5);
    if (output == NULL)
        return NULL;

    strncpy (output, source, size);
    strncat (output, ".ivt", 4);

    return output;
}

static gboolean
_setup_vibrator (Request *request)
{
    gchar   *ivtfile = NULL;

    if (request->resources & RESOURCE_VIBRATION && profile_is_vibra_enabled (request->context->profile)) {
        /* If vibrator_custom_patterns property is sent and with current ringtone file exists file with same
           name, but with .ivt extension, use that file as vibration pattern. */

        if (properties_get_bool (request->properties, "vibrator_custom_patterns") && request->audio_filename) {
            LOG_DEBUG ("Custom vibration patterns are enabled.");
            ivtfile = _get_ivt_filename (request->audio_filename);
            if (ivtfile && g_file_test (ivtfile, G_FILE_TEST_EXISTS)) {
                LOG_DEBUG ("%s: Loading vibration with custom pattern file %s.", __FUNCTION__, ivtfile);
                request->vibra_data = vibrator_load (ivtfile);
            }

            g_free (ivtfile);
            ivtfile = NULL;
        }

        return TRUE;
    }

    return FALSE;
}

static void
_shutdown_vibrator (Request *request)
{
    if (request->vibra_id > 0) {
        vibrator_stop (request->context->vibrator, request->vibra_id);
        request->vibra_id = 0;
    }
}

static gboolean
_setup_led (Request *request)
{
    const char *led = NULL;

    if ((request->resources & RESOURCE_LED) == 0)
        return FALSE;

    if ((led = properties_get_string (request->properties, "led")) != NULL)
        request->led_id = led_start (request->context->led, led);

    return TRUE;
}

static void
_shutdown_led (Request *request)
{
    if (request->led_id > 0) {
        led_stop (request->context->led, request->led_id);
        request->led_id = 0;
    }
}

static gboolean
_setup_backlight (Request *request)
{
    if ((request->resources & RESOURCE_BACKLIGHT) == 0)
        return FALSE;

    return backlight_start (request->context->backlight, properties_get_bool (request->properties, "unlock_tklock"));
}

static void
_shutdown_backlight (Request *request)
{
    if (request->context->backlight && (request->resources & RESOURCE_BACKLIGHT))
        backlight_stop (request->context->backlight);
}

gboolean
request_start (Request *request, GHashTable *properties)
{
    EventPrototype *p = request->proto;

    /* If override is allowed, make a copy of the prototype's property hash table and merge our
       custom allowed properties in. */

    request->properties = properties_copy (p->properties);
    if (!properties_get_bool (request->properties, "disallow_override")) {
        LOG_DEBUG ("Override allowed, merging properties.");
        properties_merge_allowed (request->properties, properties, p->allowed_keys);
    }
    else {
        LOG_DEBUG ("Override is not allowed.");
    }

    LOG_DEBUG ("<event properties>");
    properties_dump (request->properties);

    /* Fetch some properties and save as internal data */

    request->audio_repeat_enabled = properties_get_bool (request->properties, "audio_repeat");
    request->audio_max_repeats    = properties_get_int  (request->properties, "audio_max_repeats");
    request->audio_filename       = _get_audio_filename (request, "audio", "audio_tone_profile");

    /* Check the resources and start the backends if we have the proper resources,
       profile allows us to and valid data is provided. */

    if (properties_get_bool (request->properties, "audio_enabled")) {
        if (!_tone_generator_start (request))
            _audio_playback_start (request, TRUE);
    }

    if (properties_get_bool (request->properties, "vibra_enabled"))
        _setup_vibrator (request);

    /* Timeout callback for maximum length of the event. Once triggered we will
       stop the event ourselves. */

    guint max_length = properties_get_int (request->properties, "max_length");
    if (max_length > 0)
        request->max_length_timeout_id = g_timeout_add (max_length, _max_timeout_triggered_cb, request);

    /* Trigger the start timer, which will be used to monitor the minimum timeout. */

    g_timer_start (request->start_timer);

    return TRUE;
}

void
request_stop (Request *request)
{
    if (request->max_length_timeout_id > 0) {
        g_source_remove (request->max_length_timeout_id);
        request->max_length_timeout_id = 0;
    }

    if (request->vibra_poll_id > 0) {
        g_source_remove (request->vibra_poll_id);
        request->vibra_poll_id = 0;
    }

    _tone_generator_stop (request);
    _audio_playback_stop (request);

    _shutdown_vibrator (request);
    _shutdown_led (request);
    _shutdown_backlight (request);
}
