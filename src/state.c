#include "daemon.h"
#include "state.h"

guint
play_handler (Context *context, const char *event, GHashTable *properties)
{
    return daemon_request_play (context, event, properties);
}

void
stop_handler (Context *context, guint policy_id)
{
    daemon_request_stop (context, policy_id);
}
