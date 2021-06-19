#include "moon-draw-utils.h"
#include <algorithm>
#include "gfx_dimensions.h"
#include "moon/texts/moon-loader.h"
#include "moon/texts/text-converter.h"
#include "moon/ui/interfaces/moon-screen.h"

extern "C"{
#include "pc/platform.h"
}

float MoonGetTextWidth(std::wstring text, float scale, bool colored) {
    return (float)moon_get_text_width(Moon::GetTranslatedText(text), scale, colored);
}

float MoonGetTextWidth(std::string text, float scale, bool colored) {
    return (float)moon_get_text_width(Moon::GetTranslatedText(wide(text)), scale, colored);
}

void MoonDrawRawText(float x, float y, u8* text, float scale, struct Color color, bool dropShadow, bool u4_3){
    if(!u4_3) x = GFX_DIMENSIONS_FROM_LEFT_EDGE(x);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    if(dropShadow){
        gDPSetEnvColor(gDisplayListHead++, 10, 10, 10, 255);
        moon_draw_text(x, SCREEN_HEIGHT - y - 1 * scale, text, scale);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    }
    gDPSetEnvColor(gDisplayListHead++, color.r, color.g, color.b, color.a);
    moon_draw_text(x, SCREEN_HEIGHT - y, text, scale);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void MoonDrawWideText(float x, float y, std::wstring text, float scale, struct Color color, bool dropShadow, bool u4_3){
    MoonDrawRawText(x, y, Moon::GetTranslatedText(text), scale, color, dropShadow, u4_3);
}

void MoonDrawText(float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3){
    MoonDrawWideText(x, y, wide(text), scale, color, dropShadow, u4_3);
}

void MoonDrawRawColoredText(float x, float y, u8* text, float scale, struct Color color, bool dropShadow, bool u4_3){
    if(!u4_3) x = GFX_DIMENSIONS_FROM_LEFT_EDGE(x);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    if(dropShadow){
        moon_draw_colored_text(x, y + 1, text, scale, {10, 10, 10, 255});
    }
    gDPSetEnvColor(gDisplayListHead++, color.r, color.g, color.b, color.a);
    struct Color white = { 255, 255, 255, 255 };
    moon_draw_colored_text(x, y, text, scale, white);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void MoonDrawWideColoredText(float x, float y, std::wstring text, float scale, struct Color color, bool dropShadow, bool u4_3){
    std::transform(text.begin(), text.end(), text.begin(), ::toupper);
    MoonDrawRawColoredText(x, y, Moon::GetTranslatedText(text), scale, color, dropShadow, u4_3);
}

void MoonDrawColoredText(float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3){
    MoonDrawWideColoredText(x, y, wide(text), scale, color, dropShadow, u4_3);
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

void MoonDrawBWTexture(float x, float y, float w, float h, char* texture){
    s32 xl = MAX(0, x);
	s32 yl = MAX(0, y);
	s32 xh = MAX(0, x + w - 1);
	s32 yh = MAX(0, y + h - 1);
	s32 s = 0;
	s32 t = 0;
	gDPPipeSync(gDisplayListHead++);
	gDPSetCycleType(gDisplayListHead++, G_CYC_COPY);
	gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
	gDPSetAlphaCompare(gDisplayListHead++, G_AC_THRESHOLD);
	gDPSetBlendColor(gDisplayListHead++, 255, 255, 255, 255);
	gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gDPTileSync(gDisplayListHead++);
	gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_I, G_IM_SIZ_8b, 32, texture);
	gDPSetTile(gDisplayListHead++, G_IM_FMT_I, G_IM_SIZ_8b, 4, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0);
	gDPLoadSync(gDisplayListHead++);
	gDPLoadTile(gDisplayListHead++, 7, 0, 0, 124, 124);
	gDPPipeSync(gDisplayListHead++);
	gDPSetTile(gDisplayListHead++, G_IM_FMT_I, G_IM_SIZ_8b, 4, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0);
	gDPSetTileSize(gDisplayListHead++, 0, 0, 0, 124, 124);
	gSPTextureRectangle(gDisplayListHead++, xl << 2, yl << 2, xh << 2, yh << 2, 0, s << 5, t << 5,  4096, 1024);
	gDPPipeSync(gDisplayListHead++);
	gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
	gSPTexture(gDisplayListHead++, 65535, 65535, 0, G_TX_RENDERTILE, G_OFF);
	gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
	gDPSetAlphaCompare(gDisplayListHead++, G_AC_NONE);
	gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
}

void MoonDrawButton(int x, int y, std::string text, std::string texture, int size, int offset, bool rtl, bool u4_3){
    if(!rtl){
        MoonDrawTexture(u4_3 ? x : GFX_DIMENSIONS_FROM_LEFT_EDGE(x), y - 3 + offset, size, size, sys_strdup(texture.c_str()));
        MoonDrawText   (x + 16 + 3, y, text, 0.8, {255, 255, 255, 255}, true, u4_3);
    } else {
        x = GetScreenWidth(false) - x;
        int txtWidth = MoonGetTextWidth(text, 0.8, false);

        MoonDrawTexture(GFX_DIMENSIONS_FROM_LEFT_EDGE(x) - txtWidth - size - 3, y - 3 + offset, size, size, sys_strdup(texture.c_str()));
        MoonDrawText(x - txtWidth, y, text, 0.8, {255, 255, 255, 255}, true, false);
    }
}