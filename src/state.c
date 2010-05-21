#include "daemon.h"
#include "state.h"

guint
play_handler (Context *context, const char *event, GHashTable *properties)
{
    return daemon_event_play (context, event, properties);
}

void
stop_handler (Context *context, guint policy_id)
{
    daemon_event_stop (context, policy_id);
}
