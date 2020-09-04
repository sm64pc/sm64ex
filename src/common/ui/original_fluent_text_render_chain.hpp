#pragma once

#include <string>
#include "include/types.h"

#include "i_fluent_text_render_chain.hpp"
#include "original_text_renderer.hpp"
#include "text_to_render.hpp"

class OriginalFluentTextRenderChain : IFluentTextRenderChain {
public:
  OriginalFluentTextRenderChain(OriginalTextRenderer *parent);

  IFluentTextRenderChain* with_glyph_set(LutSource lut_source);
  IFluentTextRenderChain* with_alpha(u8 alpha);
  IFluentTextRenderChain* scheduled();
  IFluentTextRenderChain* centered();

  /* This is the final step in the chain that actually does the rendering. */
  IFluentTextRenderChain *glyphs_at(std::string text, s16 x, s16 y);
  IFluentTextRenderChain *glyphs_at(const u8 *text, s16 x, s16 y);
  IFluentTextRenderChain *glyph_at(u8 chr, s16 x, s16 y);

private:
  std::unique_ptr<OriginalTextRenderer> parent;
  TextToRender text_to_render = TextToRender();

  void trigger();
};
