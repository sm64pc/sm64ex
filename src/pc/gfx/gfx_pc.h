#ifndef GFX_PC_H
#define GFX_PC_H

#include "gfx_rendering_api_config.h"

struct GfxRenderingAPI;
struct GfxWindowManagerAPI;

struct GfxDimensions {
    uint32_t width, height;
    float aspect_ratio;
};

extern struct GfxDimensions gfx_current_dimensions;

#ifdef __cplusplus
extern "C" {
#endif

void gfx_init(struct GfxWindowManagerAPI *wapi, struct GfxRenderingAPI *rapi, const char *window_title);
struct GfxRenderingAPI *gfx_get_current_rendering_api(void);
void gfx_start_frame(void);
void gfx_run(Gfx *commands);
void gfx_end_frame(void);
void gfx_precache_textures(void);
void gfx_shutdown(void);

#ifdef GFX_SEPARATE_PROJECTIONS
void gfx_set_camera_perspective(float fov_degrees, float near_dist, float far_dist);
void gfx_set_camera_matrix(float mat[4][4]);
#endif

#ifdef GFX_ENABLE_GRAPH_NODE_MODS
void gfx_register_layout_graph_node(void *geo_layout, void *graph_node);
void *gfx_build_graph_node_mod(void *graph_node, float modelview_matrix[4][4]);
#endif

#ifdef __cplusplus
}
#endif

#endif
