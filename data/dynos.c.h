#ifndef DYNOS_C_H
#define DYNOS_C_H
#ifndef __cplusplus

#include "dynos.h"

// The action signature is "bool (*) (const char *)"
// The input is the button internal name (not label)
// The output is the result of the action
#define DYNOS_DEFINE_ACTION(func) \
DYNOS_AT_STARTUP static void dynos_opt_add_action_##func() { \
    dynos_opt_add_action(#func, func, false); \
}

s32     dynos_opt_get_value         (const char *name);
void    dynos_opt_set_value         (const char *name, s32 value);
void    dynos_opt_add_action        (const char *funcname, bool (*funcptr)(const char *), bool overwrite);

void   *dynos_update_cmd            (void *cmd);
void    dynos_update_gfx            ();
void    dynos_update_opt            (void *pad);
bool    dynos_sanity_check_geo      (s16 graphNodeType);
bool    dynos_sanity_check_seq      (u8 loBits);
s32     dynos_gfx_import_texture    (void **output, void *ptr, s32 tile, void *grapi, void **hashmap, void *pool, s32 *poolpos, s32 poolsize);
void    dynos_gfx_swap_animations   (void *ptr);

#endif
#endif
