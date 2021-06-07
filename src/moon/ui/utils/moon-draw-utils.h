#ifndef MoonDrawUtils
#define MoonDrawUtils
#include <string>

extern "C" {
#include "moon/utils/moon-gfx.h"
#include "game/ingame_menu.h"
#include "game/game_init.h"
#include "game/segment2.h"
#include "gfx_dimensions.h"
#include "config.h"
#include "game/geo_misc.h"
}

void MoonDrawRawText    (float x, float y, u8* text, float scale, struct Color color, bool dropShadow, bool u4_3);
void MoonDrawText       (float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3);
void MoonDrawWideText   (float x, float y, std::wstring text, float scale, struct Color color, bool dropShadow, bool u4_3);

float MoonGetTextWidth  (std::wstring text, float scale, bool colored);

void MoonDrawText       (float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3);
float MoonGetTextWidth  (std::string text, float scale, bool colored);

void MoonDrawRawColoredText(float x, float y, u8* text, float scale, struct Color color, bool dropShadow, bool u4_3);
void MoonDrawColoredText(float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3);
void MoonDrawWideColoredText(float x, float y, std::wstring text, float scale, struct Color color, bool dropShadow, bool u4_3);

void MoonDrawTexture    (float x, float y, float w, float h, char* texture);
void MoonDrawBWTexture  (float x, float y, float w, float h, char* texture);
void MoonDrawRectangle  (float x, float y, float w, float h, struct Color c, bool u4_3);
void MoonDrawButton     (int x, int y, std::string text, std::string texture, int size, int offset, bool rtl);

#endif
