#ifndef GFX_DIMENSIONS_H
#define GFX_DIMENSIONS_H

#include <math.h>
#include "pc/gfx/gfx_pc.h"

#define GFX_DIMENSIONS_FROM_LEFT_EDGE(v) (SCREEN_WIDTH / 2 - SCREEN_HEIGHT / 2 * gfx_current_dimensions.aspect_ratio + (v))
#define GFX_DIMENSIONS_FROM_RIGHT_EDGE(v) (SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 * gfx_current_dimensions.aspect_ratio - (v))
#define GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(v) ((int)floorf(GFX_DIMENSIONS_FROM_LEFT_EDGE(v)))
#define GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(v) ((int)ceilf(GFX_DIMENSIONS_FROM_RIGHT_EDGE(v)))
#define GFX_DIMENSIONS_ASPECT_RATIO (gfx_current_dimensions.aspect_ratio)

// If screen is taller than it is wide, radius should be equal to SCREEN_HEIGHT since we scale horizontally
#define GFX_DIMENSIONS_FULL_RADIUS (SCREEN_HEIGHT * (GFX_DIMENSIONS_ASPECT_RATIO > 1 ? GFX_DIMENSIONS_ASPECT_RATIO : 1))

#endif
