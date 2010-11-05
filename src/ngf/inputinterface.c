#include "inputinterface-internal.h"

NCore*
n_input_interface_get_core (NInputInterface *iface)
{
    if (!iface)
        return NULL;

    return iface->core;
}

int
n_input_interface_play_request (NInputInterface *iface, NRequest *request)
{
    if (!iface || !request)
        return FALSE;

    if (n_request_is_paused (request))
        return n_core_resume_request (iface->core, request);

    request->input_iface = iface;
    return n_core_play_request (iface->core, request, FALSE);
}

int
n_input_interface_pause_request (NInputInterface *iface, NRequest *request)
{
    if (!iface || !request)
        return FALSE;

    return n_core_pause_request (iface->core, request);
}

void
n_input_interface_stop_request  (NInputInterface *iface, NRequest *request)
{
    if (!iface || !request)
        return;

    n_core_stop_request (iface->core, request);
}
