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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <glib.h>

typedef enum _LogLevel
{
    LOG_LEVEL_NONE    = 0,
    LOG_LEVEL_DEBUG   = 1,
    LOG_LEVEL_INFO    = 2,
    LOG_LEVEL_REQUEST = 3,
    LOG_LEVEL_WARNING = 4,
    LOG_LEVEL_ERROR   = 5
} LogLevel;

void log_message   (LogLevel level, const char *function, int line, const char *fmt, ...);
void log_set_level (LogLevel level);

#ifdef ENABLE_DEBUG

#define LOG_REQUEST(...) \
    { log_message (LOG_LEVEL_REQUEST, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_INFO(...) \
    { log_message (LOG_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_DEBUG(...) \
    { log_message (LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_WARNING(...) \
    { log_message (LOG_LEVEL_WARNING, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_ERROR(...) \
    { log_message (LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__); }

#else

#define LOG_REQUEST(...)   do { } while (0)
#define LOG_INFO(...) do { } while (0)
#define LOG_DEBUG(...)   do { } while (0)
#define LOG_WARNING(...) do { } while (0)
#define LOG_ERROR(...)   do { } while (0)

#endif

#endif /* LOG_H */
