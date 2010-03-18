#ifndef NGF_AUDIO_STREAM_H
#define NGF_AUDIO_STREAM_H

#include <glib.h>
#include <pulse/proplist.h>

typedef struct _NgfAudioStream      NgfAudioStream;
typedef enum   _NgfAudioStreamState NgfAudioStreamState;
typedef enum   _NgfAudioStreamType  NgfAudioStreamType;

typedef void (*NgfAudioStreamCallback) (NgfAudioStream *stream, NgfAudioStreamState state, gpointer userdata);

enum _NgfAudioStreamState
{
    NGF_AUDIO_STREAM_STATE_NONE,
    NGF_AUDIO_STREAM_STATE_STARTED,
    NGF_AUDIO_STREAM_STATE_COMPLETED,
    NGF_AUDIO_STREAM_STATE_FAILED
};

enum _NgfAudioStreamType
{
    NGF_AUDIO_STREAM_NONE,
    NGF_AUDIO_STREAM_UNCOMPRESSED
};

struct _NgfAudioStream
{
    guint                   id;
    gchar                  *source;
    pa_proplist            *properties;
    NgfAudioStreamCallback  callback;
    gpointer                userdata;

    /** Private implementation data */
    NgfAudioStreamType      type;
    gpointer                iface;
    gpointer                data;
};

#endif /* NGF_AUDIO_STREAM_H */
