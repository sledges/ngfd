#include "ngf-tone-mapper.h"

struct _NgfToneMapper
{
    int dummy;
};

NgfToneMapper*
ngf_tone_mapper_create ()
{
    NgfToneMapper *self = NULL;

    if ((self = g_new0 (NgfToneMapper, 1)) == NULL) {
        goto failed;
    }

    return self;

failed:
    ngf_tone_mapper_destroy (self);
    return NULL;
}

void
ngf_tone_mapper_destroy (NgfToneMapper *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

const char*
ngf_tone_mapper_get_tone (NgfToneMapper *self, const char *uri)
{
    (void) self;
    (void) uri;

    return NULL;
}
