#ifndef NGF_AUDIO_H
#define NGF_AUDIO_H

#include <glib.h>
#include "ngf-audio-stream.h"

typedef struct _NgfAudio NgfAudio;

NgfAudio*        ngf_audio_create           ();
void             ngf_audio_destroy          (NgfAudio *self);
void             ngf_audio_set_volume       (NgfAudio *self, const char *role, gint volume);

NgfAudioStream*  ngf_audio_create_stream    (NgfAudio *self, NgfAudioStreamType type);
void             ngf_audio_destroy_stream   (NgfAudio *self, NgfAudioStream *stream);
gboolean         ngf_audio_prepare          (NgfAudio *self, NgfAudioStream *stream);
gboolean         ngf_audio_play             (NgfAudio *self, NgfAudioStream *stream);
void             ngf_audio_stop             (NgfAudio *self, NgfAudioStream *stream);

#endif /* NGF_AUDIO_H */
