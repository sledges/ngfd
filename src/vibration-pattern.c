#include "vibration-pattern.h"

VibrationPattern*
vibration_pattern_new ()
{
    return (VibrationPattern*) g_try_malloc0 (sizeof (VibrationPattern));
}

void
vibration_pattern_free (VibrationPattern *pattern)
{
    g_free (pattern->filename);
    g_free (pattern->data);
    g_free (pattern);
}

gboolean
vibration_pattern_equals (VibrationPattern *a, VibrationPattern *b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    if (a->type != b->type)
        return FALSE;

    switch (a->type) {
        case VIBRATION_PATTERN_TYPE_FILENAME:
            if (a->filename && b->filename && g_str_equal (a->filename, b->filename)) {
                if (a->pattern == b->pattern)
                    return TRUE;
            }
            break;

        case VIBRATION_PATTERN_TYPE_INTERNAL:
            if (a->pattern == b->pattern)
                return TRUE;
            break;

        default:
            break;
    }

    return FALSE;
}

void
vibration_pattern_array_free (VibrationPattern **array)
{
    VibrationPattern **iter = NULL;

    if (array == NULL)
        return;

    for (iter = array; *iter; ++iter) {
        vibration_pattern_free (*iter);
        *iter = NULL;
    }

    g_free (array);
}
