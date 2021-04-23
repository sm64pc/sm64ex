#ifndef MoonGFX
#define MoonGFX

#include "types.h"
f32 moon_get_text_width(u8* text, float scale, u8 colored);
void moon_draw_text(f32 x, f32 y, const u8 *str, float scale);

#endif