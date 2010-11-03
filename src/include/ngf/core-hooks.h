#ifndef N_CORE_HOOKS_H
#define N_CORE_HOOKS_H

#include <glib.h>
#include <ngf/request.h>
#include <ngf/proplist.h>

typedef enum _NCoreHook
{
    N_CORE_HOOK_INIT_DONE = 1,
    N_CORE_HOOK_NEW_REQUEST,
    N_CORE_HOOK_TRANSFORM_PROPERTIES,
    N_CORE_HOOK_FILTER_SINKS,
    N_CORE_HOOK_LAST
} NCoreHook;

typedef struct _NCoreHookNewRequestData
{
    NRequest *request;
} NCoreHookNewRequestData;

typedef struct _NCoreHookTransformPropertiesData
{
    NRequest  *request;
} NCoreHookTransformPropertiesData;

typedef struct _NCoreHookFilterSinksData
{
    NRequest *request;
    GList    *sinks;
} NCoreHookFilterSinksData;

const char* n_core_hook_to_string (NCoreHook hook);

#endif /* N_CORE_HOOKS_H */
