#ifndef STATE_H
#define STATE_H

#include <glib.h>
#include "context.h"

guint play_handler (Context *context, const char *event, GHashTable *properties);
void  stop_handler (Context *context, guint policy_id);

#endif /* STATE_H */
