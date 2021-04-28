#include "moon-draw-utils.h"
#include <algorithm>
#include "gfx_dimensions.h"

float MoonGetTextWidth(std::string text, float scale, bool colored) {
    return (float)moon_get_text_width(getTranslatedText(text.c_str()), scale, colored);
}

void MoonDrawText(float x, float y, std::string text, float scale, struct Color color, bool u4_3){
    if(!u4_3) x = GFX_DIMENSIONS_FROM_LEFT_EDGE(x);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, color.r, color.g, color.b, color.a);
    moon_draw_text(x, SCREEN_HEIGHT - y, getTranslatedText(text.c_str()), scale);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void MoonDrawColoredText(float x, float y, std::string text, float scale, struct Color color, bool u4_3){
    if(!u4_3) x = GFX_DIMENSIONS_FROM_LEFT_EDGE(x);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, color.r, color.g, color.b, color.a);
    std::transform(text.begin(), text.end(), text.begin(), ::toupper);
    moon_draw_colored_text(x, y, getTranslatedText(text.c_str()), scale);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void MoonDrawRectangle(float x, float y, float w, float h, struct Color c, bool u4_3){
    moon_draw_rectangle(x, y, w, h, c, u4_3);
}
