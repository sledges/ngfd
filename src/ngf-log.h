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

#ifndef NGF_LOG_H
#define NGF_LOG_H

#include <stdio.h>
#include <glib.h>

typedef enum _NgfLogCategory
{
    NGF_LOG_NONE    = 0,
    NGF_LOG_EVENT   = (1 << 0),
    NGF_LOG_MESSAGE = (1 << 1),
    NGF_LOG_DEBUG   = (1 << 2),
    NGF_LOG_WARNING = (1 << 3),
    NGF_LOG_ERROR   = (1 << 4)
} NgfLogCategory;

void    ngf_log (NgfLogCategory category, const char *function, int line, const char *fmt, ...);

#ifdef ENABLE_DEBUG

#define LOG_EVENT(...) \
    { ngf_log (NGF_LOG_EVENT, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_MESSAGE(...) \
    { ngf_log (NGF_LOG_MESSAGE, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_DEBUG(...) \
    { ngf_log (NGF_LOG_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_WARNING(...) \
    { ngf_log (NGF_LOG_WARNING, __FUNCTION__, __LINE__, __VA_ARGS__); }
#define LOG_ERROR(...) \
    { ngf_log (NGF_LOG_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__); }

#else

#define LOG_EVENT(...)   do { } while (0)
#define LOG_MESSAGE(...) do { } while (0)
#define LOG_DEBUG(...)   do { } while (0)
#define LOG_WARNING(...) do { } while (0)
#define LOG_ERROR(...)   do { } while (0)

#endif

#endif /* NGF_LOG_H */
