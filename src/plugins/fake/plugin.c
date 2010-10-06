#include <ngf/plugin.h>

#define LOG_CAT "fake: "

N_PLUGIN_NAME        ("fake")
N_PLUGIN_VERSION     ("0.1")
N_PLUGIN_AUTHOR      ("Harri Mahonen <ext-harri.mahonen@nokia.com>")
N_PLUGIN_DESCRIPTION ("Fake plugin")

N_PLUGIN_LOAD (plugin)
{
    (void) plugin;

    N_DEBUG (LOG_CAT "plugin load");
    return TRUE;
}

N_PLUGIN_UNLOAD (plugin)
{
    (void) plugin;

    N_DEBUG (LOG_CAT "plugin unload");
}
