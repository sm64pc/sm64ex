#include "original_text_renderer.hpp"

#include "config.h"
#include "game/game_init.h"
#include "game/memory.h"
#include "game/print.h"
#include "game/segment2.h"
#include "levels/menu/header.h"
#include "original_fluent_text_render_chain.hpp"

void process_rendered_strings_impl(
  std::vector<std::unique_ptr<TextToRender>>& scheduled_text);

void render_string_impl(const u8* str, s16 x, s16 y, s16 x_scale, s16 y_scale,
                        s16 dsdx, s16 dsdy,
                        u8 alpha, LutSource lut_source);

/**
 * Public
 */
void OriginalTextRenderer::render_scheduled_text() {
  process_rendered_strings_impl(scheduled_text);
}

std::unique_ptr<IFluentTextRenderChain> OriginalTextRenderer::render() {
  auto fluent_text_render_chain = (IFluentTextRenderChain*)new
    OriginalFluentTextRenderChain(this);
  return std::unique_ptr<IFluentTextRenderChain>(fluent_text_render_chain);
}

/**
 * Private
 */
void OriginalTextRenderer::render_text(const TextToRender& text_to_render) {
  render_string_impl(text_to_render.str, text_to_render.x, text_to_render.y,
                     16, // text_to_render.x_scale,
                     16, // text_to_render.y_scale,
                     1, // text_to_render.dsdx,
                     1, // text_to_render.dsdy,
                     text_to_render.alpha, text_to_render.lut_source);
}

void OriginalTextRenderer::
schedule_text(const TextToRender& text_to_render_impl) {
  scheduled_text.push_back(std::make_unique<TextToRender>(text_to_render_impl));
}

/**
 * Implementation
 */

/** Converts a char into the proper colorful glyph for the char. */
s8 char_to_glyph_index_(char c) {
  if (c >= 'A' && c <= 'Z') { return c - 55; }

  if (c >= 'a' && c <= 'z') { return c - 87; }

  if (c >= '0' && c <= '9') { return c - 48; }

  if (c == ' ') { return GLYPH_SPACE; }

  if (c == '!') {
    return GLYPH_EXCLAMATION_PNT; // !, JP only
  }

  if (c == '#') {
    return GLYPH_TWO_EXCLAMATION; // !!, JP only
  }

  if (c == '?') {
    return GLYPH_QUESTION_MARK; // ?, JP only
  }

  if (c == '&') {
    return GLYPH_AMPERSAND; // &, JP only
  }

  if (c == '%') {
    return GLYPH_PERCENT; // %, JP only
  }

  if (c == '*') {
    return GLYPH_MULTIPLY; // x
  }

  if (c == '+') {
    return GLYPH_COIN; // coin
  }

  if (c == ',') {
    return GLYPH_MARIO_HEAD; // Imagine I drew Mario's head
  }

  if (c == '-') {
    return GLYPH_STAR; // star
  }

  if (c == '.') {
    return GLYPH_PERIOD; // large shaded dot, JP only
  }

  if (c == '/') {
    return GLYPH_BETA_KEY; // beta key, JP only. Reused for Ü in EU.
  }

  return GLYPH_SPACE;
}

void add_glyph_texture(const u8* const * glyphs, s8 glyphIndex) {
  gDPPipeSync(gDisplayListHead++);
  gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1,
                     glyphs[glyphIndex]);
  gSPDisplayList(gDisplayListHead++, dl_hud_img_load_tex_block);
}

void apply_jitter(s16& x, s16& y) {
  const auto rand_xf = rand() * 1. / RAND_MAX;
  const auto rand_yf = rand() * 1. / RAND_MAX;

  x += 4 * (rand_xf - .5);
  y += 4 * (rand_yf - .5);
}

void apply_alpha_start(u8 alpha) {
  auto r = 255;
  auto g = 255;
  auto b = 255;

  /*auto r = (u8)(255 * rand() * 1. / RAND_MAX);
  auto g = (u8)(255 * rand() * 1. / RAND_MAX);
  auto b = (u8)(255 * rand() * 1. / RAND_MAX);*/

  if (alpha != 255) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, r, g, b, alpha);
  }
}

void apply_alpha_end(u8 alpha) {
  if (alpha != 255) { gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end); }
}

const u8* const * get_glyphs(LutSource lut_source) {
  switch (lut_source) {
    case LutSource::JAPANESE:
      // Japanese Menu HUD Color font
      return (const u8*const *)segmented_to_virtual(menu_hud_lut);
    case LutSource::DEFAULT:
    default:
      // 0-9 A-Z HUD Color Font
      return (const u8*const *)segmented_to_virtual(main_hud_lut);
  }
}

u32 get_x_stride(const LutSource lut_source) {
  if (lut_source == LutSource::JAPANESE) { return 16; }
  else {
    // HUD_LUT_GLOBAL
#if defined(VERSION_JP)
    return 14;
#else
    return 12; //? Shindou uses this.
#endif
  }
}

void apply_stride(u8 chr, s16& x, const u32 x_stride) {
#ifdef VERSION_EU
  if (chr == GLOBAL_CHAR_SPACE) {
    x += x_stride / 2;
    return;
  }
#endif
#if defined(VERSION_US) || defined(VERSION_SH)
  if (chr == GLOBAL_CHAR_SPACE) {
    x += 8;
    return;
  }
#endif

  x += x_stride;
}

#ifndef WIDESCREEN
/**
 * Clips textrect into the boundaries defined.
 */
void clip_to_bounds(s16& x, s16& y) {
  if (x < TEXRECT_MIN_X) { x = TEXRECT_MIN_X; }

  if (x > TEXRECT_MAX_X) { x = TEXRECT_MAX_X; }

  if (y < TEXRECT_MIN_Y) { y = TEXRECT_MIN_Y; }

  if (y > TEXRECT_MAX_Y) { y = TEXRECT_MAX_Y; }
}
#endif

void render_textrect(s16 x, s16 y) {
  //apply_jitter(x, y);

#ifndef WIDESCREEN
  // For widescreen we must allow drawing outside the usual area
  clip_to_bounds(x, y);
#endif
  gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + 16 - 1) << 2,
                      (y + 16 - 1) << 2,
                      G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10);
}

void process_rendered_strings_impl(
  std::vector<std::unique_ptr<TextToRender>>& scheduled_text) {

  auto text_to_render_count = scheduled_text.size();
  if (text_to_render_count == 0) { return; }

  Mtx* mtx;

  mtx = (Mtx*)alloc_display_list(sizeof(*mtx));
  if (mtx == NULL) {
    scheduled_text.clear();
    return;
  }

  guOrtho(mtx, 0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -10.0f, 10.0f, 1.0f);
  gSPPerspNormalize((Gfx *) (gDisplayListHead++), 0xFFFF);
  gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(mtx),
            G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPDisplayList(gDisplayListHead++, dl_hud_img_begin);

  for (auto i = 0; i < text_to_render_count; ++i) {
    auto& text_to_render = *scheduled_text[i];

    const auto str = text_to_render.str;
    const auto length = text_to_render.length;

    auto cur_x = text_to_render.x;
    const auto cur_y = text_to_render.y;

    auto lut_source = text_to_render.lut_source;

    const auto glyphs = get_glyphs(lut_source);
    const auto x_stride = get_x_stride(lut_source);

    apply_alpha_start(text_to_render.alpha);

    for (auto j = 0; j < length; j++) {
      auto chr = str[j];
      // auto glyphIndex = char_to_glyph_index_(chr);
      // Assumes it's already formatted in glyphs.
      auto glyphIndex = chr;

      // if (glyphIndex != GLYPH_SPACE) {
#ifdef VERSION_EU
      // Beta Key was removed by EU, so glyph slot reused.
      // This produces a colorful Ü.
      if (glyphIndex == GLYPH_BETA_KEY) {
        add_glyph_texture(GLYPH_U);
        render_textrect(text_to_render);

        add_glyph_texture(GLYPH_UMLAUT);
        render_textrect(sTextLabels[i]->x, sTextLabels[i]->y + 3, j);
      } else {
        add_glyph_texture(glyphIndex);
        render_textrect(text_to_render);
      }
#endif
#if defined(VERSION_US) || defined(VERSION_SH)
      if (chr != GLOBAL_CHAR_SPACE) {
#endif

      add_glyph_texture(glyphs, glyphIndex);
      render_textrect(cur_x, cur_y);

      cur_x += x_stride;
#if defined(VERSION_US) || defined(VERSION_SH)
      }
#endif
      //}

      apply_stride(chr, cur_x, x_stride);
    }

    apply_alpha_end(text_to_render.alpha);
  }

  gSPDisplayList(gDisplayListHead++, dl_hud_img_end);

  scheduled_text.clear();
}

void render_char_tile(s16 x, s16 y, s16 x_scale, s16 y_scale, s16 dsdx,
                      s16 dsdy) {
  // apply_jitter(x, y);

  gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + x_scale) << 2,
                      (y + y_scale) << 2,
                      G_TX_RENDERTILE, 0, 0, dsdx << 10, dsdy << 10);
}

#ifdef VERSION_EU
void print_hud_char_umlaut(u8 chr, s16 x, s16 y, s16 x_scale, s16 y_scale, s16 dsdx, s16 dsdy) {
  void **fontLUT = segmented_to_virtual(main_hud_lut);

  gDPPipeSync(gDisplayListHead++);
  render_char_tile(chr, x, y, x_scale, y_scale, dsdx, dsdy, fontLUT);
  render_char_tile(chr, x, y - y_scale / 16. * 4, x_scale, y_scale, dsdx, dsdy, fontLUT);
}
#endif

void render_string_impl(const u8* str, s16 x, s16 y, s16 x_scale, s16 y_scale,
                        s16 dsdx, s16 dsdy,
                        u8 alpha, LutSource lut_source = LutSource::DEFAULT) {
  s32 str_pos = 0;
  auto cur_x = x;
  const auto cur_y = y;

  const auto lut = get_glyphs(lut_source);
  auto x_stride = get_x_stride(lut_source);

  apply_alpha_start(alpha);

  while (str[str_pos] != GLOBAR_CHAR_TERMINATOR) {
    const auto chr = str[str_pos];

#ifdef VERSION_EU
    switch (chr) {
      case GLOBAL_CHAR_SPACE:
        break;
      case HUD_CHAR_A_UMLAUT:
        print_hud_char_umlaut(ASCII_TO_DIALOG('A'), cur_x, cur_y, x_scale, y_scale, dsdx, dsdy);
        break;
      case HUD_CHAR_O_UMLAUT:
        print_hud_char_umlaut(ASCII_TO_DIALOG('O'), cur_x, cur_y, x_scale, y_scale, dsdx, dsdy);
        break;
      case HUD_CHAR_U_UMLAUT:
        print_hud_char_umlaut(ASCII_TO_DIALOG('U'), cur_x, cur_y, x_scale, y_scale, dsdx, dsdy);
        break;
      default:
#endif
#if defined(VERSION_US) || defined(VERSION_SH)
        if (chr != GLOBAL_CHAR_SPACE) {
#endif

    gDPPipeSync(gDisplayListHead++);
    add_glyph_texture(lut, chr);
    render_char_tile(cur_x, cur_y, x_scale, y_scale, dsdx, dsdy);

#ifdef VERSION_EU
          break;
        }
#endif
#if defined(VERSION_US) || defined(VERSION_SH)
    }
#endif

    apply_stride(chr, cur_x, x_stride);

    str_pos++;
  }

  apply_alpha_end(alpha);
}
