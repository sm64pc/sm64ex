#ifndef MoonGFX
#define MoonGFX

#include "types.h"
f32 moon_get_text_width(u8* text, float scale, u8 colored);
void moon_draw_colored_text(f32 x, f32 y, const u8 *str, float scale);
void moon_draw_text(f32 x, f32 y, const u8 *str, float scale);
void moon_draw_rectangle(s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b);
void moon_draw_texture(s32 x, s32 y, u32 w, u32 h, u8 *texture);

#endif