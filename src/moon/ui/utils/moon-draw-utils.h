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
#include "text/txtconv.h"
}

void MoonDrawText       (float x, float y, std::string text, float scale, struct Color color, bool u4_3);
void MoonDrawColoredText(float x, float y, std::string text, float scale, struct Color color, bool u4_3);
void MoonDrawRectangle  (float x, float y, float w, float h, struct Color c, bool u4_3);

#endif
