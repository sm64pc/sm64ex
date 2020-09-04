#include "service_locator.hpp"

#include "common/ui/original_text_renderer.hpp"

/**
 * Public
 */
ITextRenderer& ServiceLocator::get_text_renderer() { return *text_renderer; }

/**
 * Private
 */
ITextRenderer* ServiceLocator::text_renderer = new OriginalTextRenderer();