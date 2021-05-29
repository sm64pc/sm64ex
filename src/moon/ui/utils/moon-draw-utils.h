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

void MoonDrawText       (float x, float y, std::wstring text, float scale, struct Color color, bool dropShadow, bool u4_3);
float MoonGetTextWidth  (std::wstring text, float scale, bool colored);
void MoonDrawColoredText(float x, float y, std::wstring text, float scale, struct Color color, bool dropShadow, bool u4_3);

void MoonDrawText       (float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3);
float MoonGetTextWidth  (std::string text, float scale, bool colored);
void MoonDrawColoredText(float x, float y, std::string text, float scale, struct Color color, bool dropShadow, bool u4_3);

void MoonDrawTexture    (float x, float y, float w, float h, char* texture);
void MoonDrawRectangle  (float x, float y, float w, float h, struct Color c, bool u4_3);

#endif
