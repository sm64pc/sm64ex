#ifndef GFX_RENDERING_API_H
#define GFX_RENDERING_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef RAPI_RT64
#   define GFX_MAX_BUFFERED                 16384
#   define GFX_DISABLE_SHADOWS
#   define GFX_DISABLE_FRUSTUM_CULLING
#   define GFX_DISABLE_LIGHTING
#   define GFX_DISABLE_CLIP_REJECT
#   define GFX_FLUSH_ON_ENDDL
#   define GFX_OUTPUT_NORMALS_TO_VBO
#   define GFX_SEPARATE_PROJECTIONS
#   define GFX_SEPARATE_FOG
#   define GFX_REQUIRE_TEXTURE_HASH
#   define GFX_ENABLE_GRAPH_NODE_MODS
#endif

struct ShaderProgram;

struct GfxRenderingAPI {
    bool (*z_is_from_0_to_1)(void);
    void (*unload_shader)(struct ShaderProgram *old_prg);
    void (*load_shader)(struct ShaderProgram *new_prg);
    struct ShaderProgram *(*create_and_load_new_shader)(uint32_t shader_id);
    struct ShaderProgram *(*lookup_shader)(uint32_t shader_id);
    void (*shader_get_info)(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]);
#ifndef GFX_REQUIRE_TEXTURE_HASH
    uint32_t (*new_texture)(void);
#else
    uint32_t (*new_texture)(uint64_t hash);
#endif
    void (*select_texture)(int tile, uint32_t texture_id);
    void (*upload_texture)(const uint8_t *rgba32_buf, int width, int height);
    void (*set_sampler_parameters)(int sampler, bool linear_filter, uint32_t cms, uint32_t cmt);
    void (*set_depth_test)(bool depth_test);
    void (*set_depth_mask)(bool z_upd);
    void (*set_zmode_decal)(bool zmode_decal);
    void (*set_viewport)(int x, int y, int width, int height);
    void (*set_scissor)(int x, int y, int width, int height);
    void (*set_use_alpha)(bool use_alpha);
#ifdef GFX_SEPARATE_FOG
    void (*set_fog)(uint8_t fog_r, uint8_t fog_g, uint8_t fog_b, int16_t fog_mul, int16_t fog_offset);
#endif
#ifndef GFX_SEPARATE_PROJECTIONS
    void (*draw_triangles)(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris);
#else
    void (*set_camera_config)(float fov_degrees, float near_dist, float far_dist);
    void (*set_camera_vectors)(float pos_x, float pos_y, float pos_z, float focus_x, float focus_y, float focus_z, float up_x, float up_y, float up_z);
    void (*draw_triangles_ortho)(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris);
    void (*draw_triangles_persp)(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris, float transform_affine[4][4]);
#endif
#ifdef GFX_ENABLE_GRAPH_NODE_MODS
    void (*push_geo_layout)(void *geo_layout);
    void (*register_graph_node_layout)(void *graph_node);
    void (*pop_geo_layout)(void);
    void *(*get_graph_node_mod)(void *graph_node);
    void (*set_graph_node_mod)(void *graph_node_mod);
#endif
    void (*init)(void);
    void (*on_resize)(void);
    void (*start_frame)(void);
    void (*end_frame)(void);
    void (*finish_render)(void);
    void (*shutdown)(void);
};

#endif
