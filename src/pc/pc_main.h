#ifndef _PC_MAIN_H
#define _PC_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

extern struct GfxWindowManagerAPI* wm_api;
void game_deinit(void);
void game_exit(void);

#ifdef __cplusplus
}
#endif

#endif // _PC_MAIN_H
