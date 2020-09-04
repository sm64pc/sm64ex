#include "original_fluent_text_render_chain.hpp"

#include "original_text_renderer.hpp"

OriginalFluentTextRenderChain::OriginalFluentTextRenderChain(
  OriginalTextRenderer* parent) : parent(parent) {}

IFluentTextRenderChain* OriginalFluentTextRenderChain::with_glyph_set(
  LutSource lut_source) {
  text_to_render.lut_source = lut_source;
  return this;
}

IFluentTextRenderChain* OriginalFluentTextRenderChain::with_alpha(u8 alpha) {
  text_to_render.alpha = alpha;
  return this;
}

IFluentTextRenderChain* OriginalFluentTextRenderChain::scheduled() {
  text_to_render.scheduled = true;
  return this;
}

IFluentTextRenderChain* OriginalFluentTextRenderChain::centered() {
  text_to_render.centered = true;
  return this;
}

void OriginalFluentTextRenderChain::trigger() {
  if (text_to_render.scheduled) {
    parent->schedule_text(text_to_render);
  } else {
    parent->render_text(text_to_render);
  }
}

IFluentTextRenderChain *OriginalFluentTextRenderChain::glyphs_at(std::string text, s16 x, s16 y) {
  text_to_render.set_glyphs(text);
  text_to_render.x = x;
  text_to_render.y = y;

  trigger();

  return this;
}

IFluentTextRenderChain *OriginalFluentTextRenderChain::glyphs_at(const u8 *text, s16 x, s16 y) {
  text_to_render.set_glyphs(text);
  text_to_render.x = x;
  text_to_render.y = y;

  trigger();

  return this;
}

IFluentTextRenderChain *OriginalFluentTextRenderChain::glyph_at(u8 chr, s16 x, s16 y) {
  text_to_render.set_glyph(chr);
  text_to_render.x = x;
  text_to_render.y = y;

  trigger();

  return this;
}
