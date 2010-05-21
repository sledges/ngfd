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

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "config.h"

#ifdef HAVE_TIMESTAMP
    #include <sp_timestamp.h>
    #define TIMESTAMP(A)    sp_timestamp(A);
#else
    #define TIMESTAMP(A)
#endif /* HAVE_TIMESTAMP */


#endif /* TIMESTAMP_H */
