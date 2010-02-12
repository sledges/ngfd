#include "ngf-event-definition.h"

NgfEventDefinition*
ngf_event_definition_new ()
{
    return (NgfEventDefinition*) g_new0 (NgfEventDefinition, 1);
}

void
ngf_event_definition_free (NgfEventDefinition *self)
{
    if (self == NULL)
        return;

    g_free (self->long_proto);
    g_free (self->short_proto);
    g_free (self);
}
