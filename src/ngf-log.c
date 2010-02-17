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

#include <stdarg.h>
#include <stdlib.h>
#include "ngf-log.h"

static const char*
category_to_string (NgfLogCategory category)
{
    switch (category) {
        case NGF_LOG_EVENT:
            return "EVENT";

        case NGF_LOG_MESSAGE:
            return "MESSAGE";

        case NGF_LOG_DEBUG:
            return "DEBUG";

        case NGF_LOG_WARNING:
            return "WARNING";

        case NGF_LOG_ERROR:
            return "ERROR";

        default:
            break;
    }

    return "UNKNOWN";
}

void
ngf_log (NgfLogCategory category, const char *function, int line, const char *fmt, ...)
{
    char buf[256];

    (void) function;
    (void) line;

    va_list fmt_args;
    va_start (fmt_args, fmt);
    vsnprintf (buf, 256, fmt, fmt_args);
    va_end (fmt_args);

    fprintf (stdout, "[%s] %s\n", category_to_string (category), buf);
}
