/*
 * ngfd - Non-graphic feedback daemon
 *
 * Copyright (C) 2010 Nokia Corporation.
 * Contact: Xun Chen <xun.chen@nokia.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
