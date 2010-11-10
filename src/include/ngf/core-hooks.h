#ifndef N_CORE_HOOKS_H
#define N_CORE_HOOKS_H

#include <glib.h>
#include <ngf/request.h>
#include <ngf/proplist.h>

typedef enum _NCoreHook
{
    /** Executed after:
    - All plugins are loaded
    - All the NSinkInterface's and NInputInterface's have been 
    initialized.
    */
    N_CORE_HOOK_INIT_DONE = 1,
    /** Executed:
    - After the core receive's a new request from the NInputInterface.
    - Before event has been resolved.
    - Before any transforming, modifications.
    - Example is transform-plugin, which allows only certain properties to 
    go through and discards the rest.
    */
    N_CORE_HOOK_NEW_REQUEST,
    /** Executed:
    - After event has been resolved for the request and properties have been 
    merged.
    - Profile plugins transforms the *.profile keys to their target keys.
    - If somebody would like to have a "theme" based support, then this 
    would be the hook to act on.
    */
    N_CORE_HOOK_TRANSFORM_PROPERTIES,
    /** Executed:
    - Core asks from the sinks if they can handle the request.
    - This is called after that to allow extra filtering.
    - Example is resource plugin, which enables/disables sinks for request 
    based on available resources (and for example, vibration status).
    */
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

/**
 * Return name of hook as string
 *
 * @param hook Hook.
 * @return Name of the hook.
 */
const char* n_core_hook_to_string (NCoreHook hook);

#endif /* N_CORE_HOOKS_H */
