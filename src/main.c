/*
 * ngfd - Non-graphical feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation. All rights reserved.
 *
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating,
 * any or all of this material requires the prior written consent of
 * Nokia Corporation. This material also contains confidential
 * information which may not be disclosed to others without the prior
 * written consent of Nokia.
 */

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
