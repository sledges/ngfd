#include "volume.h"

Volume*
volume_new ()
{
    Volume *v = NULL;

    v = (Volume*) g_try_malloc0 (sizeof (Volume));
    return v;
}

void
volume_free (Volume *v)
{
    g_free (v->key);
    g_free (v->profile);

    if (v->controller) {
        controller_free (v->controller);
        v->controller = NULL;
    }

    g_free (v);
}

gboolean
volume_equals (Volume *a, Volume *b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type) {
        case VOLUME_TYPE_FIXED:
            if (a->level == b->level)
                return TRUE;

            break;

        case VOLUME_TYPE_PROFILE:
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

        case VOLUME_TYPE_CONTROLLER:
            if (a->controller && b->controller && g_str_equal (a->controller, b->controller))
                return TRUE;

            break;

        default:
            break;
    }

    return FALSE;
}
