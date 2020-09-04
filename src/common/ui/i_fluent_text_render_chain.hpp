#pragma once

#include "include/types.h"
#include "lut_source.hpp"

class IFluentTextRenderChain {
public:
  virtual IFluentTextRenderChain *with_glyph_set(LutSource lut_source) = 0;
  virtual IFluentTextRenderChain *with_alpha(u8 alpha) = 0;
  virtual IFluentTextRenderChain *scheduled() = 0;
  virtual IFluentTextRenderChain *centered() = 0;

  /* This is the final step in the chain that actually does the rendering. */
  virtual IFluentTextRenderChain *glyphs_at(std::string text, s16 x, s16 y) = 0;
  virtual IFluentTextRenderChain *glyphs_at(const u8 *text, s16 x, s16 y) = 0;
  virtual IFluentTextRenderChain *glyph_at(u8 chr, s16 x, s16 y) = 0;
};