#include <glib.h>
#include "ngf-daemon.h"
#include "ngf-value.h"

int
main (int argc, char *argv[])
{
    NgfDaemon *daemon = NULL;

    if ((daemon = ngf_daemon_create ()) == NULL)
        return 1;

    ngf_daemon_run (daemon);
    ngf_daemon_destroy (daemon);

    return 0;
}
