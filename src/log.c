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
#include "log.h"

static LogLevel _log_level = LOG_LEVEL_INFO;

static const char*
level_to_string (LogLevel category)
{
    switch (category) {
        case LOG_LEVEL_REQUEST:
            return "REQUEST";

        case LOG_LEVEL_INFO:
            return "INFO";

        case LOG_LEVEL_DEBUG:
            return "DEBUG";

        case LOG_LEVEL_WARNING:
            return "WARNING";

        case LOG_LEVEL_ERROR:
            return "ERROR";

        default:
            break;
    }

    return "UNKNOWN";
}

void
log_message (LogLevel category, const char *function, int line, const char *fmt, ...)
{
    char buf[256];

    (void) function;
    (void) line;

    if (category < _log_level)
        return;

    va_list fmt_args;
    va_start (fmt_args, fmt);
    vsnprintf (buf, 256, fmt, fmt_args);
    va_end (fmt_args);

    fprintf (stdout, "[%s] %s\n", level_to_string (category), buf);
}

void
log_set_level (LogLevel level)
{
    _log_level = level;
}
