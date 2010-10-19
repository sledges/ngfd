#include "log.h"
#include "core-hooks.h"

const char*
n_core_hook_to_string (NCoreHook hook)
{
    switch (hook) {
        case N_CORE_HOOK_INIT_DONE:
            return "init_done";
        case N_CORE_HOOK_TRANSFORM_PROPERTIES:
            return "transform_properties";
        case N_CORE_HOOK_FILTER_SINKS:
            return "filter_sinks";
        default:
            break;
    }

    return "unknown";
}
