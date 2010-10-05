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

#ifndef N_LOG_H
#define N_LOG_H

#include <stdarg.h>

typedef enum _NLogTarget
{
    N_LOG_TARGET_NONE = 0,
    N_LOG_TARGET_STDOUT,
    N_LOG_TARGET_SYSLOG
} NLogTarget;

typedef enum _NLogLevel
{
    N_LOG_LEVEL_ENTER   = 0,
    N_LOG_LEVEL_DEBUG   = 1,
    N_LOG_LEVEL_INFO    = 2,
    N_LOG_LEVEL_WARNING = 3,
    N_LOG_LEVEL_ERROR   = 4,
    N_LOG_LEVEL_NONE    = 5
} NLogLevel;

void n_log_initialize (NLogLevel level);
void n_log_set_level  (NLogLevel level);
void n_log_set_target (NLogTarget target);
void n_log_message    (NLogLevel level, const char *function, int line, const char *fmt, ...);

#define N_ENTER(...) \
    { n_log_message (N_LOG_LEVEL_ENTER, (const char*) __FUNCTION__, __LINE__, __VA_ARGS__); }
#define N_DEBUG(...) \
    { n_log_message (N_LOG_LEVEL_DEBUG, (const char*) __FUNCTION__, __LINE__, __VA_ARGS__); }
#define N_INFO(...) \
    { n_log_message (N_LOG_LEVEL_INFO, (const char*) __FUNCTION__, __LINE__, __VA_ARGS__); }
#define N_WARNING(...) \
    { n_log_message (N_LOG_LEVEL_WARNING, (const char*) __FUNCTION__, __LINE__, __VA_ARGS__); }
#define N_ERROR(...) \
    { n_log_message (N_LOG_LEVEL_ERROR, (const char*) __FUNCTION__, __LINE__, __VA_ARGS__); }

#endif /* N_LOG_H */
