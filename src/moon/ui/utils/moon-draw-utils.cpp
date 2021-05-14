#include "moon-draw-utils.h"
#include <algorithm>
#include "gfx_dimensions.h"

float MoonGetTextWidth(std::string text, float scale, bool colored) {
    return (float)moon_get_text_width(getTranslatedText(text.c_str()), scale, colored);
}

void MoonDrawText(float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3){
    if(!u4_3) x = GFX_DIMENSIONS_FROM_LEFT_EDGE(x);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    if(dropShadow){
        gDPSetEnvColor(gDisplayListHead++, 10, 10, 10, 255);
        moon_draw_text(x, SCREEN_HEIGHT - y - 1 * scale, getTranslatedText(text.c_str()), scale);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    }
    gDPSetEnvColor(gDisplayListHead++, color.r, color.g, color.b, color.a);
    moon_draw_text(x, SCREEN_HEIGHT - y, getTranslatedText(text.c_str()), scale);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void MoonDrawColoredText(float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3){
    if(!u4_3) x = GFX_DIMENSIONS_FROM_LEFT_EDGE(x);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    std::transform(text.begin(), text.end(), text.begin(), ::toupper);
    if(dropShadow){
        moon_draw_colored_text(x, y + 1, getTranslatedText(text.c_str()), scale, {10, 10, 10, 255});
    }
    gDPSetEnvColor(gDisplayListHead++, color.r, color.g, color.b, color.a);
    struct Color white = { 255, 255, 255, 255 };
    moon_draw_colored_text(x, y, getTranslatedText(text.c_str()), scale, white);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void MoonDrawRectangle(float x, float y, float w, float h, struct Color c, bool u4_3){
    moon_draw_rectangle(x, y, w, h, c, u4_3);
}

void MoonDrawTexture(float x, float y, float w, float h, char* texture){
    gSPDisplayList(gDisplayListHead++, dl_hud_img_begin);
    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD);
    gDPTileSync(gDisplayListHead++);
    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 2, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR, 3, G_TX_NOLOD, G_TX_NOMIRROR, 3, G_TX_NOLOD);
    gDPSetTileSize(gDisplayListHead++, G_TX_RENDERTILE, 0, 0, (int)w << G_TEXTURE_IMAGE_FRAC, (int)h << G_TEXTURE_IMAGE_FRAC);
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 1, texture);
    gDPLoadSync(gDisplayListHead++);
    gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, w * h - 1, CALC_DXT(w, G_IM_SIZ_32b_BYTES));
    gSPTextureRectangle(gDisplayListHead++, (int) x << 2, (int) y << 2, (int)(x + w) << 2, (int)(y + h) << 2, G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10);
    gSPDisplayList(gDisplayListHead++, dl_hud_img_end);
}