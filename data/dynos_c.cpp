#include "dynos.cpp.h"
extern "C" {

s32 dynos_opt_get_value(const char *name) {
    return DynOS_Opt_GetValue(name);
}

void dynos_opt_set_value(const char *name, s32 value) {
    return DynOS_Opt_SetValue(name, value);
}

void dynos_opt_add_action(const char *funcname, bool (*funcptr)(const char *), bool overwrite) {
    return DynOS_Opt_AddAction(funcname, funcptr, overwrite);
}

void *dynos_update_cmd(void *cmd) {
    return DynOS_UpdateCmd(cmd);
}

void dynos_update_gfx() {
    return DynOS_UpdateGfx();
}

void dynos_update_opt(void *pad) {
    return DynOS_UpdateOpt(pad);
}

s32 dynos_gfx_import_texture(void **output, void *ptr, s32 tile, void *grapi, void **hashmap, void *pool, s32 *poolpos, s32 poolsize) {
    return DynOS_Gfx_ImportTexture(output, ptr, tile, grapi, hashmap, pool, (u32 *) poolpos, (u32) poolsize);
}

void dynos_gfx_swap_animations(void *ptr) {
    return DynOS_Gfx_SwapAnimations(ptr);
}

}
