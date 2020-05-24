#ifndef CONTROLLER_SDL_H
#define CONTROLLER_SDL_H

#include "controller_api.h"

#define VK_BASE_SDL_GAMEPAD 0x1000

extern struct ControllerAPI controller_sdl;

s32 controller_rumble_init(void);
s32 controller_rumble_play(f32 strength, u32 length);
s32 controller_rumble_stop(void);

#endif
