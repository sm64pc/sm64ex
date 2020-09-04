#pragma once

#include <memory>
#include <vector>

#include "i_text_renderer.hpp"
#include "include/types.h"
#include "text_to_render.hpp"

class OriginalFluentTextRenderChain;

class OriginalTextRenderer : public ITextRenderer {
  friend OriginalFluentTextRenderChain;

public:
  void render_scheduled_text();

  std::unique_ptr<IFluentTextRenderChain> render();

private:
  void render_text(const TextToRender &text_to_render);
  void schedule_text(const TextToRender &text_to_render);

  std::vector<std::unique_ptr<TextToRender>> scheduled_text;
};
