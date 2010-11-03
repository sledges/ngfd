#ifndef N_HOOK_H
#define N_HOOK_H

#include <glib.h>

typedef enum _NHookPriority
{
    N_HOOK_PRIORITY_LAST    = -100,
    N_HOOK_PRIORITY_LOW     = -10,
    N_HOOK_PRIORITY_DEFAULT = 0,
    N_HOOK_PRIORITY_HIGH    = 10,
    N_HOOK_PRIORITY_FIRST   = 100
} NHookPriority;

typedef struct _NHook
{
    gchar *name;
    GList *slots;
} NHook;

typedef void (*NHookCallback) (NHook *hook, void *data, void *userdata);

void n_hook_init       (NHook *hook);
void n_hook_clear      (NHook *hook);

int  n_hook_connect    (NHook *hook, int priority, NHookCallback callback, void *userdata);
void n_hook_disconnect (NHook *hook, NHookCallback callback, void *userdata);
int  n_hook_fire       (NHook *hook, void *data);

#endif /* N_HOOK_H */
