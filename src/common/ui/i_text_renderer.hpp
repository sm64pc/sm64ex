#pragma once

#include <memory>
#include <string>
#include "include/types.h"

#include "i_fluent_text_render_chain.hpp"

class ITextRenderer {
public:
  virtual void render_scheduled_text() = 0;

  virtual std::unique_ptr<IFluentTextRenderChain> render() = 0;
};
