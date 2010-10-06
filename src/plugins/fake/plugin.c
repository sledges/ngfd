#include <ngf/plugin.h>

#define LOG_CAT "fake: "

N_PLUGIN_NAME        ("fake")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_AUTHOR      ("Harri Mahonen <ext-harri.mahonen@nokia.com>")
N_PLUGIN_DESCRIPTION ("Fake plugin")

static int
fake_sink_initialize (NSinkInterface *iface)
{
    (void) iface;
    N_DEBUG (LOG_CAT "sink initialize");
    return TRUE;
}

static void
fake_sink_shutdown (NSinkInterface *iface)
{
    (void) iface;
    N_DEBUG (LOG_CAT "sink shutdown");
}

static int
fake_sink_can_handle (NSinkInterface *iface /*, NRequest *request */)
{
    (void) iface;
    N_DEBUG (LOG_CAT "sink can_handle");
    return TRUE;
}

static int
fake_sink_prepare (NSinkInterface *iface /*, NRequest *request */)
{
    (void) iface;
    N_DEBUG (LOG_CAT "sink prepare");
    return TRUE;
}

static int
fake_sink_play (NSinkInterface *iface /*, NRequest *request */)
{
    (void) iface;
    N_DEBUG (LOG_CAT "sink play");
    return TRUE;
}

static void
fake_sink_stop (NSinkInterface *iface /*, NRequest *request */)
{
    (void) iface;
    N_DEBUG (LOG_CAT "sink stop");
}

N_PLUGIN_LOAD (plugin)
{
    N_DEBUG (LOG_CAT "plugin load");

    static const NSinkInterfaceDecl decl = {
        .name       = "fake",
        .initialize = fake_sink_initialize,
        .shutdown   = fake_sink_shutdown,
        .can_handle = fake_sink_can_handle,
        .prepare    = fake_sink_prepare,
        .play       = fake_sink_play,
        .stop       = fake_sink_stop
    };

    n_plugin_register_sink (plugin, &decl);

    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    (void) plugin;

    N_DEBUG (LOG_CAT "plugin unload");
}
