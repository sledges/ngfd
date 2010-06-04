#include "audio.h"
#include "volume-controller.h"

int
volume_controller_update (Context *context, Volume *volume)
{
    if (!context || !volume)
        return FALSE;

    if (!volume->role)
        return FALSE;

    audio_set_volume (context->audio, volume->role, volume->level);
    return TRUE;
}

int
volume_controller_update_all (Context *context)
{
    Volume **i = NULL;

    if (!context || (context && !context->volumes))
        return FALSE;

    for (i = context->volumes; *i; ++i)
        volume_controller_update (context, *i);

    return TRUE;
}
