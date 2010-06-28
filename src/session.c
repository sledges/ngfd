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

#include <glib.h>
#include <gio/gio.h>

#include <unistd.h>
#include <sys/inotify.h>

#include "log.h"
#include "profile.h"
#include "context.h"
#include "session.h"

#define SESSION_BUS_ADDRESS_FILENAME "/tmp/session_bus_address.user"
#define EVENT_SIZE                   (sizeof (struct inotify_event))
#define BUF_LEN                      (1024 * (EVENT_SIZE + 16))
#define POLL_TIMEOUT                 5

static gint        inotify_fd      = 0;
static gint        inotify_wd      = 0;
static GIOChannel *inotify_channel = NULL;
static guint       inotify_source  = 0;
static guint       poll_id         = 0;

static gboolean    poll_session_bus_address_file (gpointer userdata);
static void        setup_file_polling            (Context *context);
static gboolean    setup_file_watcher            (Context *context);
static void        stop_file_polling             ();
static void        close_file_watcher            ();
static gchar*      parse_address                 (const char *buf);
static gchar*      load_session_bus_address      (const char *filename);
static gboolean    session_new_bus               (Context *context, const char *address);
static void        session_reconnect             (Context *context);



static gboolean
poll_session_bus_address_file (gpointer userdata)
{
    Context *context = (Context*) userdata;
    gchar   *address = NULL;

    if (g_file_test (SESSION_BUS_ADDRESS_FILENAME, G_FILE_TEST_EXISTS)) {
        LOG_DEBUG ("%s >> session bus address file exists.", __FUNCTION__);

        if ((address = load_session_bus_address (SESSION_BUS_ADDRESS_FILENAME)) != NULL) {
            stop_file_polling  ();
            session_new_bus    (context, address);
            g_free             (address);
            setup_file_watcher (context);
            return FALSE;
        }
    }

    return TRUE;
}

static void
setup_file_polling (Context *context)
{
    LOG_DEBUG ("%s >> polling for session bus address file every %d seconds.", __FUNCTION__, POLL_TIMEOUT);

    if (poll_id > 0) {
        g_source_remove (poll_id);
        poll_id = 0;
    }

    poll_id = g_timeout_add_seconds (POLL_TIMEOUT, poll_session_bus_address_file, context);
}

static void
stop_file_polling ()
{
    if (poll_id > 0) {
        LOG_DEBUG ("%s >> stopping session bus address file polling.", __FUNCTION__);
        g_source_remove (poll_id);
        poll_id = 0;
    }
}

static gboolean
inotify_watch (GIOChannel *source, GIOCondition condition, gpointer userdata)
{
    Context *context = (Context*) userdata;

    char buf[BUF_LEN];
    int len, i = 0, fd = 0;
    gchar *address = NULL;
    gboolean success = FALSE;

    struct inotify_event *event = NULL;

    (void) userdata;

    if (condition & G_IO_IN) {

        fd = g_io_channel_unix_get_fd (source);
        len = read (fd, buf, BUF_LEN);

        while (i < len) {
            event = (struct inotify_event *) &buf[i];

            if (event->mask & IN_DELETE_SELF) {

                LOG_DEBUG ("%s >> session bus address file was removed.", __FUNCTION__);
                close_file_watcher ();
                setup_file_polling (context);

                return FALSE;
            }

            if ((event->mask & IN_MODIFY)) {

                success = FALSE;

                if ((address = load_session_bus_address (SESSION_BUS_ADDRESS_FILENAME)) != NULL) {
                    if (session_new_bus (context, address)) {
                        session_reconnect (context);
                        success = TRUE;
                    }
                }

                if (!success) {
                    close_file_watcher ();
                    setup_file_polling (context);
                    return FALSE;
                }
            }

            i += EVENT_SIZE + event->len;
        }

    }

    return TRUE;
}

static gboolean
setup_file_watcher (Context *context)
{
    if ((inotify_fd = inotify_init ()) < 0)
        goto failed;

    if ((inotify_wd = inotify_add_watch (inotify_fd, SESSION_BUS_ADDRESS_FILENAME, IN_MODIFY | IN_ATTRIB | IN_DELETE_SELF)) < 0)
        goto failed;

    if ((inotify_channel = g_io_channel_unix_new (inotify_fd)) == NULL)
        goto failed;

    if ((inotify_source = g_io_add_watch (inotify_channel, G_IO_IN | G_IO_PRI, inotify_watch, context)) == 0)
        goto failed;

    return TRUE;

failed:
    close_file_watcher ();
    return FALSE;
}

static void
close_file_watcher ()
{
    if (inotify_source > 0) {
        g_source_remove (inotify_source);
        inotify_source = 0;
    }

    if (inotify_channel) {
        g_io_channel_unref (inotify_channel);
        inotify_channel = NULL;
    }

    if (inotify_wd >= 0) {
        inotify_rm_watch (inotify_fd, inotify_wd);
        inotify_wd = 0;
    }

    if (inotify_fd >= 0) {
        close (inotify_fd);
        inotify_fd = 0;
    }
}

static gchar*
parse_address (const char *buf)
{
    gchar **lines   = NULL;
    gchar **tokens  = NULL;
    gchar **i       = NULL;
    gchar  *address = NULL;

    if (!buf)
        return NULL;

    if ((lines = g_strsplit (buf, "\n", 2)) == NULL)
        return NULL;

    for (i = lines; i; ++i) {
        if ((tokens = g_strsplit (*i, "=", 2)) == NULL)
            continue;

        if (!tokens[0]) {
            g_strfreev (tokens);
            continue;
        }

        if (tokens[0] && tokens[1]) {
            if (g_str_has_suffix (tokens[0], "DBUS_SESSION_BUS_ADDRESS")) {
                address = g_strdup (tokens[1]);
                g_strfreev (tokens);
                return address;
            }
        }

        g_strfreev (tokens);
    }

    g_strfreev (lines);

    return NULL;
}

static gchar*
load_session_bus_address (const char *filename)
{
    FILE   *fp         = NULL;
    long    file_size  = 0;
    size_t  bytes_read = 0;
    char   *buf        = NULL;
    gchar  *address    = NULL;

    if (!filename)
        return NULL;

    if ((fp = fopen (filename, "r")) == NULL) {
        LOG_DEBUG ("%s >> no address file %s", __FUNCTION__, filename);
        return NULL;
    }

    fseek (fp, 0L, SEEK_END);
    file_size = ftell (fp);
    fseek (fp, 0L, SEEK_SET);

    if (file_size > 0) {
        if ((buf = (char*) g_try_malloc0 (sizeof (char) * file_size)) == NULL) {
            LOG_WARNING ("%s >> failed to allocate memory to read session bus address file.", __FUNCTION__);
            fclose (fp);
            return NULL;
        }

        bytes_read = fread (buf, sizeof (char), file_size, fp);
        if (bytes_read != file_size) {
            LOG_WARNING ("%s >> failed to read the session bus address file.", __FUNCTION__);
            g_free (buf);
            fclose (fp);
            return NULL;
        }

        if ((address = parse_address (buf)) != NULL) {
            LOG_DEBUG ("%s >> parsed DBus session bus address: %s", __FUNCTION__, address);
        }
        else {
            LOG_WARNING ("%s >> failed to parse DBus session bus address.", __FUNCTION__);
        }

        g_free (buf);
    }

    fclose (fp);

    return address;
}

static gboolean
session_new_bus (Context *context, const char *address)
{
    DBusError error;

    dbus_error_init (&error);
    if (context->session_bus != NULL) {
        LOG_DEBUG ("%s >> received session bus address '%s'", __FUNCTION__, address);
        dbus_connection_unref (context->session_bus);
        context->session_bus = NULL;
    }
    else
        LOG_DEBUG ("%s >> received new session bus address '%s'", __FUNCTION__, address);

    if ((context->session_bus = dbus_connection_open (address, &error)) == NULL ||
        !dbus_bus_register (context->session_bus, &error))
    {
        if (dbus_error_is_set(&error)) {
            LOG_WARNING ("%s >> failed to connect to session bus %s (%s)", __FUNCTION__, address, error.message);
            dbus_error_free (&error);
        }
        else
            LOG_WARNING ("%s >> failed to connect to session bus %s", __FUNCTION__, address);

        return FALSE;
    }

    session_reconnect (context);

    return TRUE;
}

static void
session_reconnect (Context *context)
{
    profile_reconnect     (context);
    tone_mapper_reconnect (context);
}

int
session_create (Context *context)
{
    gchar *address = NULL;

    /* if the session bus address file exists already, then try to load the address. */

    if (g_file_test (SESSION_BUS_ADDRESS_FILENAME, G_FILE_TEST_EXISTS)) {
        if ((address = load_session_bus_address (SESSION_BUS_ADDRESS_FILENAME)) != NULL) {
            session_new_bus    (context, address);
            g_free             (address);

            setup_file_watcher (context);
        }
    }
    else {
        setup_file_polling (context);
    }

    return TRUE;
}

void
session_destroy (Context *context)
{
    stop_file_polling  (context);
    close_file_watcher (context);
}
