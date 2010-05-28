#include <string.h>

#include "log.h"
#include "event.h"
#include "resources.h"
#include "player.h"

#define AUDIO_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_AUDIO)

#define VIBRATION_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_VIBRATION)

#define LEDS_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_LEDS)

#define BACKLIGHT_RESOURCE_ENABLED(request) \
    RESOURCE_ENABLED(request->resources, RESOURCE_BACKLIGHT)

static gboolean prepare_stream                   (Request *request, SoundPath *sound_path);
static gboolean play_stream                      (Request *request);
static void     stop_stream                      (Request *request);
static gboolean play_vibration                   (Request *request, VibrationPattern *pattern);
static void     stop_vibration                   (Request *request);

static gboolean playback_path_tone_generator     (Request *request);
static gboolean playback_path_synchronize        (Request *request);
static gboolean playback_path_vibration          (Request *request);
static gboolean playback_path_leds_and_backlight (Request *request);
static gboolean playback_path_no_sound           (Request *request);



static gboolean
max_timeout_cb (gpointer userdata)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Event   *event   = request->event;
    guint    timeout = 0;

    if (request->max_timeout_id > 0)
        g_source_remove (request->max_timeout_id);

    timeout = (event->max_timeout > 0 && (event->max_timeout < request->play_timeout)) ? event->max_timeout : request->play_timeout;
    if (timeout <= 0)
        return;

    LOG_DEBUG ("%s >> set to %d milliseconds", __FUNCTION__, timeout);
    request->max_timeout_id = g_timeout_add (timeout, max_timeout_cb, request);
}

static void
remove_timeout (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    if (request->max_timeout_id > 0) {
        g_source_remove (request->max_timeout_id);
        request->max_timeout_id = 0;
    }
}

static gboolean
controller_set_volume (Controller *controller, guint id, guint t, guint v, gpointer userdata)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Request *request = (Request*) userdata;
    Context *context = request->context;
    Event   *event   = request->event;

    (void) id;
    (void) t;

    audio_set_volume (context->audio, event->stream_role, v);
    return TRUE;
}

static void
set_stream_volume (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;
    Volume  *volume  = event->volume;

    if (!volume)
        return;

    switch (volume->type) {
        case VOLUME_TYPE_FIXED:
        case VOLUME_TYPE_PROFILE:
            audio_set_volume (context->audio, event->stream_role, volume->level);
            break;

        case VOLUME_TYPE_CONTROLLER:
            request->controller_id = controller_start (volume->controller, controller_set_volume, request);
            break;

        default:
            break;
    }
}

static void
clear_stream_volume (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;
    Volume  *volume  = event->volume;


    if (request->controller_id > 0) {
        controller_stop (volume->controller, request->controller_id);
        request->controller_id = 0;
    }
}

static const gchar*
get_uncompressed_tone (ToneMapper *mapper, const char *tone)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    const char *uncompressed = NULL;

    if (mapper == NULL || tone == NULL)
        return NULL;

    uncompressed = tone_mapper_get_tone (mapper, tone);
    if (uncompressed) {
        LOG_DEBUG ("Tone (uncompressed): %s", uncompressed);
        return uncompressed;
    }

    return NULL;
}

static SoundPath*
resolve_sound_path (Request *request, gboolean advance)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Event     *event      = request->event;
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
        sound_path = (SoundPath*) request->sound_iterator->data;
    }

    if (!sound_path)
        return NULL;

    LOG_DEBUG ("%s >> (type=%d, filename=%s, key=%s, profile=%s)", __FUNCTION__,
        sound_path->type, sound_path->filename, sound_path->key, sound_path->profile);

    return sound_path;
}

static gchar*
build_vibration_filename (const char *source)
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

static VibrationPattern*
resolve_custom_pattern (Request *request, SoundPath *sound_path)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context          *context  = request->context;
    Event            *event    = request->event;
    VibrationPattern *pattern  = NULL;
    gpointer          data     = NULL;
    gchar            *filename = NULL;

    if (!sound_path || (sound_path && !sound_path->filename))
        return NULL;

    if ((filename = build_vibration_filename (sound_path->filename)) == NULL)
        return NULL;

    if ((data = vibrator_load (filename)) == NULL) {
        g_free (filename);
        return NULL;
    }

    pattern = vibration_pattern_new ();
    pattern->type     = VIBRATION_PATTERN_TYPE_FILENAME;
    pattern->filename = filename;
    pattern->data     = data;

    LOG_DEBUG ("%s >> custom pattern (type=%d, filename=%s, pattern=%d)", __FUNCTION__,
        pattern->type, pattern->filename, pattern->pattern);

    return pattern;
}

static VibrationPattern*
resolve_vibration_pattern (Request *request, gboolean advance)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Event            *event   = request->event;
    VibrationPattern *pattern = NULL;

    if (advance)
        request->vibration_iterator = g_list_next (request->vibration_iterator);

    if (request->vibration_iterator == NULL)
        return NULL;

    pattern = (VibrationPattern*) request->vibration_iterator->data;

    LOG_DEBUG ("%s >> pattern (type=%d, filename=%s, pattern=%d)", __FUNCTION__,
        pattern->type, pattern->filename, pattern->pattern);

    return pattern;
}

static void
synchronize_resources (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context  *context = request->context;
    Event    *event   = request->event;
    gboolean  success = FALSE;

    if (!request->synchronize_done) {
        request->synchronize_done = TRUE;

        setup_timeout                    (request);
        set_stream_volume                (request);
        playback_path_leds_and_backlight (request);
        playback_path_vibration          (request);
        play_stream                      (request);

        return;
    }

    if (request->active_pattern && request->custom_pattern)
        (void) play_vibration (request, request->active_pattern);

    (void) play_stream (request);
}

static gboolean
restart_next_stream (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    SoundPath *sound_path = NULL;
    stop_stream (request);

    if ((sound_path = resolve_sound_path (request, TRUE)) == NULL)
        return FALSE;

    return prepare_stream (request, sound_path);
}

static gboolean
repeat_current_stream (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Event     *event      = request->event;
    SoundPath *sound_path = NULL;

    if (request->custom_pattern)
        stop_vibration (request);

    stop_stream (request);

    if (!event->repeat)
        return FALSE;

    if (event->num_repeats == 0 || (request->repeat_count < event->num_repeats)) {
        request->repeat_count++;

        if ((sound_path = resolve_sound_path (request, FALSE)) == NULL)
            return FALSE;

        return prepare_stream (request, sound_path);
    }

    return FALSE;
}

static void
stream_state_cb (AudioStream *stream, AudioStreamState state, gpointer userdata)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Request *request        = (Request*) userdata;
    Context *context        = request->context;
    Event   *event          = request->event;
    guint    callback_state = REQUEST_STATE_NONE;

    SoundPath *sound_path = NULL;

    switch (state) {
        case AUDIO_STREAM_STATE_PREPARED:
            LOG_DEBUG ("%s >> prepared", __FUNCTION__);
            synchronize_resources (request);
            break;

        case AUDIO_STREAM_STATE_STARTED:
            LOG_DEBUG ("%s >> started", __FUNCTION__);
            callback_state = REQUEST_STATE_STARTED;
            break;

        case AUDIO_STREAM_STATE_FAILED:
            LOG_DEBUG ("%s >> failed", __FUNCTION__);
            if (!restart_next_stream (request))
                callback_state = REQUEST_STATE_FAILED;
            break;

        case AUDIO_STREAM_STATE_COMPLETED:
            LOG_DEBUG ("%s >> completed", __FUNCTION__);
            if (!repeat_current_stream (request)) {

                callback_state = REQUEST_STATE_COMPLETED;
                if (event->leds_enabled && LEDS_RESOURCE_ENABLED (request)) {
                   callback_state = REQUEST_STATE_UPDATED;
                   remove_timeout (request);
                }
            }

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context     *context       = request->context;
    Event       *event         = request->event;
    const gchar *uncompressed  = NULL;
    gchar       *stream_source = NULL;
    guint        stream_type   = 0;
    AudioStream *stream        = NULL;

    /* tone mapper provides an uncompressed file for us to play
       if available. */

    uncompressed = get_uncompressed_tone (context->tone_mapper, sound_path->filename);
    if (uncompressed) {
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
    stream->properties     = pa_proplist_copy (event->stream_properties);
    stream->callback       = stream_state_cb;
    stream->userdata       = request;

    request->stream       = stream;
    request->active_sound = sound_path;

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    
    if (request->stream) {
        audio_stop (context->audio, request->stream);
        audio_destroy_stream (context->audio, request->stream);
        request->stream = NULL;
    }
}

static gboolean
play_vibration (Request *request, VibrationPattern *pattern)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;
 
    if (!pattern)
        return FALSE;

    if ((request->vibration_id = vibrator_start (context->vibrator, pattern->data, pattern->pattern)) > 0)
        return TRUE;

    return FALSE;
}

static void
stop_vibration (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    if (request->vibration_id > 0) {
        vibrator_stop (context->vibrator, request->vibration_id);
        request->vibration_id = 0;
    }
}

static gboolean
playback_path_tone_generator (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    tone_generator_start (context->system_bus, event->tone_generator_pattern);
    request->tone_generator_active = TRUE;

    return TRUE;
}

static gboolean
playback_path_synchronize (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    if (event->leds_enabled && LEDS_RESOURCE_ENABLED (request)) {
        led_activate_pattern (context->system_bus, event->led_pattern);
        request->leds_active = TRUE;
    }

    if (event->backlight_enabled && BACKLIGHT_RESOURCE_ENABLED (request)) {

        if (event->unlock_tklock)
            backlight_unlock_tklock (context->system_bus);

        backlight_display_on (context->system_bus);
        backlight_prevent_blank (context->system_bus);
        request->backlight_active = TRUE;
    }

    return TRUE;
}

static gboolean
playback_path_no_sound (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    setup_timeout                    (request);
    playback_path_vibration          (request);
    playback_path_leds_and_backlight (request);

    return TRUE;
}

int
play_request (Request *request)
{
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

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
    LOG_DEBUG ("%s >> entering", __FUNCTION__);

    Context *context = request->context;
    Event   *event   = request->event;

    if (request->backlight_active) {
        backlight_cancel_prevent_blank (context->system_bus);
        request->backlight_active = FALSE;
    }

    if (request->leds_active) {
        led_deactivate_pattern (context->system_bus, event->led_pattern);
        request->leds_active = FALSE;
    }

    if (request->custom_pattern) {
        vibration_pattern_free (request->custom_pattern);
        request->custom_pattern = NULL;
    }

    remove_timeout      (request);
    clear_stream_volume (request);
    stop_vibration      (request);
    stop_stream         (request);
}
