#include <glib.h>
#include "ngf-audio.h"

typedef struct _AudioApp
{
    GMainLoop   *loop;
    NgfAudio    *audio;
} AudioApp;

static void
_audio_state_cb (NgfAudio *audio, NgfAudioState state, gpointer userdata)
{
    AudioApp *app = (AudioApp*) userdata;

    gboolean quit = FALSE;

    switch (state) {
        case NGF_AUDIO_READY:
            g_print ("READY\n");
            break;

        case NGF_AUDIO_FAILED:
            g_print ("FAILED\n");
            quit = TRUE;
            break;

        case NGF_AUDIO_TERMINATED:
            g_print ("TERMINATED\n");
            quit = TRUE;
            break;

        case NGF_AUDIO_SAMPLE_LIST: {
            g_print ("SAMPLE LIST\n");

            GList *iter = NULL, *sample_list = NULL;

            sample_list = ngf_audio_get_sample_list (app->audio);
            for (iter = sample_list; iter; iter = g_list_next (iter)) {
                g_print ("Cached sample = %s\n", (gchar*) iter->data);
            }
            ngf_audio_free_sample_list (sample_list);

            break;
        }

        default:
            break;
    }

    if (quit) {
        g_print ("QUIT\n");
        g_main_loop_quit (app->loop);
    }
}

int
main (int argc, char *argv[])
{
    AudioApp app = { 0 };

    app.loop = g_main_loop_new (NULL, 0);
    app.audio = ngf_audio_create ();
    ngf_audio_set_callback (app.audio, _audio_state_cb, &app);

    g_main_loop_run (app.loop);

    ngf_audio_destroy (app.audio);
    g_main_loop_unref (app.loop);

    return 0;
}
