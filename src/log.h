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
