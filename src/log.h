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

typedef enum _LogCategory
{
    LOG_CATEGORY_NONE    = 0,
    LOG_CATEGORY_EVENT   = (1 << 0),
    LOG_CATEGORY_MESSAGE = (1 << 1),
    LOG_CATEGORY_DEBUG   = (1 << 2),
    LOG_CATEGORY_WARNING = (1 << 3),
    LOG_CATEGORY_ERROR   = (1 << 4)
} LogCategory;

void log_message (LogCategory category, const char *function, int line, const char *fmt, ...);

#ifdef ENABLE_DEBUG

#define LOG_EVENT(...) \
    { log_message (LOG_CATEGORY_EVENT, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_MESSAGE(...) \
    { log_message (LOG_CATEGORY_MESSAGE, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_DEBUG(...) \
    { log_message (LOG_CATEGORY_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_WARNING(...) \
    { log_message (LOG_CATEGORY_WARNING, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_ERROR(...) \
    { log_message (LOG_CATEGORY_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__); }

#else

#define LOG_EVENT(...)   do { } while (0)
#define LOG_MESSAGE(...) do { } while (0)
#define LOG_DEBUG(...)   do { } while (0)
#define LOG_WARNING(...) do { } while (0)
#define LOG_ERROR(...)   do { } while (0)

#endif

#endif /* LOG_H */
