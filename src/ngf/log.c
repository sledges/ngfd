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
#include <signal.h>
#include <syslog.h>
#include "log.h"

#ifdef ENABLE_DEBUG
static LogLevel _log_level = LOG_LEVEL_ENTER;
#else
static LogLevel _log_level = LOG_LEVEL_NONE;
#endif

static gboolean _log_syslog = FALSE;

static const char*
level_to_string (LogLevel category)
{
    switch (category) {
        case LOG_LEVEL_ENTER:
            return "ENTER";

        case LOG_LEVEL_INFO:
            return "INFO";

        case LOG_LEVEL_DEBUG:
            return "DEBUG";

        case LOG_LEVEL_WARNING:
            return "WARNING";

        default:
            break;
    }

    return "UNKNOWN";
}

static int
level_to_syslevel (LogLevel category)
{
    switch (category) {
        case LOG_LEVEL_ENTER:
            return LOG_DEBUG;

        case LOG_LEVEL_INFO:
            return LOG_INFO;

        case LOG_LEVEL_DEBUG:
            return LOG_DEBUG;

        case LOG_LEVEL_WARNING:
            return LOG_WARNING;

        default:
            break;
    }

    return LOG_INFO;
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

    if (_log_syslog)
        syslog (level_to_syslevel (category), "[%s] %s\n", level_to_string (category), buf);
    else
        fprintf (stdout, "[%s] %s\n", level_to_string (category), buf);
}

void
log_signal (int signum, siginfo_t *info, void *ptr)
{
    (void) signum;
    (void) info;
    (void) ptr;

    if (!_log_syslog) {
        _log_syslog = TRUE;
        openlog ("ngfd", 0, LOG_DAEMON);
    } else {
        closelog();
        _log_syslog = FALSE;
    }

    _log_level = LOG_LEVEL_ENTER;
    NGF_LOG_INFO ("Logging enabled");
}

void
log_set_level (LogLevel level)
{
    _log_level = level;

    if (level < LOG_LEVEL_NONE) {
        if (_log_syslog)
            syslog (LOG_INFO, "Log level set to %s\n", level_to_string (level));
        else
            fprintf (stdout, "Log level set to %s\n", level_to_string (level));
    }
}
