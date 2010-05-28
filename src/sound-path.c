#include "sound-path.h"

SoundPath*
sound_path_new ()
{
    SoundPath *s = NULL;

    s = (SoundPath*) g_try_malloc0 (sizeof (SoundPath));
    return s;
}

void
sound_path_free (SoundPath *s)
{
    g_free (s->filename);
    g_free (s->key);
    g_free (s->profile);
    g_free (s);
}

gboolean
sound_path_equals (SoundPath *a, SoundPath *b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type) {
        case SOUND_PATH_TYPE_FILENAME:
            if (a->filename && b->filename && g_str_equal (a->filename, b->filename))
                return TRUE;
            break;

        case SOUND_PATH_TYPE_PROFILE:
            if (a->key == NULL || b->key == NULL)
                return FALSE;

            if (g_str_equal (a->key, b->key)) {
                if (a->profile && b->profile && g_str_equal (a->profile, b->profile))
                    return TRUE;

                if (a->profile == NULL && b->profile == NULL)
                    return TRUE;

                return FALSE;
            }

            break;

        default:
            break;
    }

    return FALSE;
}

void
sound_path_array_free (SoundPath **array)
{
    SoundPath **iter = NULL;

    if (array == NULL)
        return;

    for (iter = array; *iter; ++iter) {
        sound_path_free (*iter);
        *iter = NULL;
    }

    g_free (array);
}
