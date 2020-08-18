#ifndef GFX_PC_H
#define GFX_PC_H

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

#ifdef __cplusplus
}
#endif

#endif
