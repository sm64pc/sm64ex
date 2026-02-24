#ifndef GFX_SDL_H
#define GFX_SDL_H

#include "gfx_window_manager_api.h"

#ifdef DECLARE_GFX_SDL_FUNCTIONS
extern "C" void *gfx_sdl_get_layer();
#endif

extern struct GfxWindowManagerAPI gfx_sdl;

#endif
