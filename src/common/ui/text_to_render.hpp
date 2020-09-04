#pragma once

#include "include/types.h"

#include "game/ingame_menu.h"
#include "i_text_renderer.hpp"

/* Helper class for bundling information about text to render. */
class TextToRender {
public:
  TextToRender() { }

  void set_glyphs(std::string text) {
    length = (s16)text.length();
    for (auto i = 0; i < length; ++i) { str[i] = text[i]; }
    str[length] = GLOBAR_CHAR_TERMINATOR;
  }

  void set_glyphs(const u8* str) {
    u8 c;
    s32 strPos = 0;
    do {
      c = str[strPos];
      this->str[strPos] = c;

      strPos++;
    } while (c != GLOBAR_CHAR_TERMINATOR);
    length = strPos - 1;
  }

  void set_glyph(u8 chr) {
    str[0] = chr;
    str[1] = GLOBAR_CHAR_TERMINATOR;
    length = 1;
  }

  TextToRender(const TextToRender& other) {
    u8 c;
    s32 strPos = 0;
    do {
      c = other.str[strPos];
      this->str[strPos] = c;

      strPos++;
    } while (c != GLOBAR_CHAR_TERMINATOR);
    length = strPos - 1;

    this->x = other.x;
    this->y = other.y;
  }

  u8 str[50];
  s16 length;

  s16 x;
  s16 y;

  u8 alpha = 255;

  bool centered = false;
  bool scheduled = false;

  // s16 x_scale = 16;
  // s16 y_scale = 16;

  // s16 dsdx = 4;
  // s16 dsdy = 1;

  LutSource lut_source;
};
