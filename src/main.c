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
#include "daemon.h"

int
main (int argc, char *argv[])
{
    Context *context = NULL;

    if ((context = daemon_create ()) == NULL)
        return 1;

    daemon_run (context);
    daemon_destroy (context);

    return 0;
}
