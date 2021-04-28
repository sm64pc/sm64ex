#ifndef MoonGFX
#define MoonGFX

#include "types.h"

struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

#define COLOR(red, green, blue, alpha) \
    { .r = red, .g = green, .b = blue, .a = alpha }

f32 moon_get_text_width(u8* text, float scale, u8 colored);
void moon_draw_colored_text(f32 x, f32 y, const u8 *str, float scale);
void moon_draw_text(f32 x, f32 y, const u8 *str, float scale);
void moon_draw_rectangle(f32 x, f32 y, f32 w, f32 h, struct Color c, u8 u4_3);
void moon_draw_texture(s32 x, s32 y, u32 w, u32 h, u8 *texture);

#endif