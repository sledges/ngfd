#ifndef NGF_AUDIO_INTERFACE_H
#define NGF_AUDIO_INTERFACE_H

#include <glib.h>

#include "ngf-pulse-context.h"
#include "ngf-audio-stream.h"

typedef struct _NgfAudioInterface NgfAudioInterface;

struct _NgfAudioInterface
{
    gboolean (*initialize) (NgfAudioInterface *iface, NgfPulseContext *context);
    void     (*shutdown)   (NgfAudioInterface *iface);
    gboolean (*prepare)    (NgfAudioInterface *iface, NgfAudioStream *stream);
    gboolean (*play)       (NgfAudioInterface *iface, NgfAudioStream *stream);
    void     (*stop)       (NgfAudioInterface *iface, NgfAudioStream *stream);

    gpointer data;
};

gboolean        ngf_audio_interface_initialize     (NgfAudioInterface *iface, NgfPulseContext *context);
void            ngf_audio_interface_shutdown       (NgfAudioInterface *iface);
NgfAudioStream* ngf_audio_interface_create_stream  (NgfAudioInterface *iface);
void            ngf_audio_interface_destroy_stream (NgfAudioInterface *iface, NgfAudioStream *stream);
gboolean        ngf_audio_interface_prepare        (NgfAudioInterface *iface, NgfAudioStream *stream);
gboolean        ngf_audio_interface_play           (NgfAudioInterface *iface, NgfAudioStream *stream);
void            ngf_audio_interface_stop           (NgfAudioInterface *iface, NgfAudioStream *stream);

#endif /* NGF_AUDIO_INTERFACE_H */
