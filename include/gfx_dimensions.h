#ifndef GFX_DIMENSIONS_H
#define GFX_DIMENSIONS_H

#ifndef TARGET_N64
#include <math.h>
#include "pc/gfx/gfx_pc.h"
#define GFX_DIMENSIONS_FROM_LEFT_EDGE(v) (SCREEN_WIDTH / 2 - SCREEN_HEIGHT / 2 * gfx_current_dimensions.aspect_ratio + (v))
#define GFX_DIMENSIONS_FROM_RIGHT_EDGE(v) (SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 * gfx_current_dimensions.aspect_ratio - (v))
#define GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(v) floorf(GFX_DIMENSIONS_FROM_LEFT_EDGE(v))
#define GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(v) ceilf(GFX_DIMENSIONS_FROM_RIGHT_EDGE(v))
#define GFX_DIMENSIONS_ASPECT_RATIO (gfx_current_dimensions.aspect_ratio)
#else
#define GFX_DIMENSIONS_FROM_LEFT_EDGE(v) (v)
#define GFX_DIMENSIONS_FROM_RIGHT_EDGE(v) (SCREEN_WIDTH - (v))
#define GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(v) (v)
#define GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(v) (SCREEN_WIDTH - (v))
#define GFX_DIMENSIONS_ASPECT_RATIO (4.0f / 3.0f)
#endif

#endif
