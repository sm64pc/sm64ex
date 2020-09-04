#pragma once

#include "common/ui/i_text_renderer.hpp"

class ServiceLocator {
public:
  static ITextRenderer &get_text_renderer();

private:
  static ITextRenderer* text_renderer;
};