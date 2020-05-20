#ifdef LEGACY_GL

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#ifdef __MINGW32__
#define FOR_WINDOWS 1
#else
#define FOR_WINDOWS 0
#endif

#include <SDL2/SDL.h>

#if FOR_WINDOWS || defined(OSX_BUILD)
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

#include "gfx_cc.h"
#include "gfx_rendering_api.h"
#include "macros.h"

enum MixFlags {
    SH_MF_OVERRIDE_ALPHA = 1,

    SH_MF_MULTIPLY = 2,
    SH_MF_MIX = 4,
    SH_MF_SINGLE = 8,

    SH_MF_MULTIPLY_ALPHA = 16,
    SH_MF_MIX_ALPHA = 32,
    SH_MF_SINGLE_ALPHA = 64,

    SH_MF_INPUT_ALPHA = 128,
};

enum MixType {
    SH_MT_NONE,
    SH_MT_TEXTURE,
    SH_MT_COLOR,
    SH_MT_TEXTURE_TEXTURE,
    SH_MT_TEXTURE_COLOR,
    SH_MT_COLOR_COLOR,
};

struct ShaderProgram {
    uint32_t shader_id;
    enum MixType mix;
    uint32_t mix_flags;
    bool texture_used[2];
    int num_inputs;
};

static struct ShaderProgram shader_program_pool[64];
static uint8_t shader_program_pool_size;
static struct ShaderProgram *cur_shader = NULL;

static const float *cur_buf = NULL;
static size_t cur_buf_size = 0;
static size_t cur_buf_num_tris = 0;
static size_t cur_buf_stride = 0;

static const float c_white[] = { 1.f, 1.f, 1.f, 1.f };

static bool gfx_opengl_z_is_from_0_to_1(void) {
    return false;
}

#define TEXENV_COMBINE_ON() glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE)
#define TEXENV_COMBINE_OFF() glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)

#define TEXENV_COMBINE_OP(num, cval, aval) \
    do { \
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND ## num ## _RGB, cval); \
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND ## num ## _ALPHA, aval); \
    } while (0)

#define TEXENV_COMBINE_SET1(what, mode, val) \
    do { \
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ ## what, mode); \
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ ## what, val); \
    } while (0)

#define TEXENV_COMBINE_SET2(what, mode, val1, val2) \
    do { \
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ ## what, mode); \
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ ## what, val1); \
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ ## what, val2); \
    } while (0)

#define TEXENV_COMBINE_SET3(what, mode, val1, val2, val3) \
    do { \
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ ## what, mode); \
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ ## what, val1); \
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ ## what, val2); \
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ ## what, val3); \
    } while (0)

static inline void texenv_set_texture_color(struct ShaderProgram *prg) {
    glActiveTexture(GL_TEXTURE0);

    if (prg->mix_flags & SH_MF_OVERRIDE_ALPHA) {
        TEXENV_COMBINE_ON();
        if (prg->mix_flags & SH_MF_SINGLE_ALPHA) {
            if (prg->mix_flags & SH_MF_MULTIPLY) {
                // keep the alpha but modulate the color
                const GLenum alphasrc = (prg->mix_flags & SH_MF_INPUT_ALPHA) ? GL_PRIMARY_COLOR : GL_TEXTURE;
                TEXENV_COMBINE_SET2(RGB, GL_MODULATE, GL_TEXTURE, GL_PRIMARY_COLOR);
                TEXENV_COMBINE_SET1(ALPHA, GL_REPLACE, alphasrc);
            } else {
                // somehow makes it keep the color while taking the alpha from primary color
                TEXENV_COMBINE_SET1(RGB, GL_REPLACE, GL_TEXTURE);
            }
        } else { // if (prg->mix_flags & SH_MF_SINGLE) {
            if (prg->mix_flags & SH_MF_MULTIPLY_ALPHA) {
                // modulate the alpha but keep the color
                TEXENV_COMBINE_SET2(ALPHA, GL_MODULATE, GL_TEXTURE, GL_PRIMARY_COLOR);
                TEXENV_COMBINE_SET1(RGB, GL_REPLACE, GL_TEXTURE);
            } else {
                // somehow makes it keep the alpha
                TEXENV_COMBINE_SET1(ALPHA, GL_REPLACE, GL_TEXTURE);
            }
        }
        // TODO: MIX and the other one
    } else if (prg->mix_flags & SH_MF_MULTIPLY) {
        // TODO: is this right?
        TEXENV_COMBINE_OFF();
    } else if (prg->mix_flags & SH_MF_MIX) {
        TEXENV_COMBINE_ON();
        // HACK: determine this using flags and not this crap
        if (prg->num_inputs > 1) {
            // out.rgb = mix(color0.rgb, color1.rgb, texel0.rgb);
            // no color1 tho, so mix with white (texenv color is set in init())
            TEXENV_COMBINE_OP(2, GL_SRC_COLOR, GL_SRC_ALPHA);
            TEXENV_COMBINE_SET3(RGB, GL_INTERPOLATE, GL_CONSTANT, GL_PRIMARY_COLOR, GL_TEXTURE);
            TEXENV_COMBINE_SET1(ALPHA, GL_REPLACE, GL_CONSTANT);
        } else {
            // out.rgb = mix(color0.rgb, texel0.rgb, texel0.a);
            TEXENV_COMBINE_OP(2, GL_SRC_ALPHA, GL_SRC_ALPHA);
            TEXENV_COMBINE_SET3(RGB, GL_INTERPOLATE, GL_TEXTURE, GL_PRIMARY_COLOR, GL_TEXTURE);
        }
    } else {
        TEXENV_COMBINE_OFF();
    }
}

static inline void texenv_set_texture_texture(UNUSED struct ShaderProgram *prg) {
    glActiveTexture(GL_TEXTURE0);
    TEXENV_COMBINE_OFF();
    glActiveTexture(GL_TEXTURE1);
    TEXENV_COMBINE_ON();
    // out.rgb = mix(texel0.rgb, texel1.rgb, color0.rgb);
    TEXENV_COMBINE_OP(2, GL_SRC_COLOR, GL_SRC_ALPHA);
    TEXENV_COMBINE_SET3(RGB, GL_INTERPOLATE, GL_PREVIOUS, GL_TEXTURE, GL_PRIMARY_COLOR);
    // out.a = texel0.a;
    TEXENV_COMBINE_SET1(ALPHA, GL_REPLACE, GL_PREVIOUS);
}

static void gfx_opengl_apply_shader(struct ShaderProgram *prg) {
    const float *ofs = cur_buf;

    // vertices are always there
    glVertexPointer(4, GL_FLOAT, cur_buf_stride, ofs);
    ofs += 4;

    // have texture(s), specify same texcoords for every active texture
    for (int i = 0; i < 2; ++i) {
        if (prg->texture_used[i]) {
            glEnable(GL_TEXTURE0 + i);
            glClientActiveTexture(GL_TEXTURE0 + i);
            glActiveTexture(GL_TEXTURE0 + i);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, cur_buf_stride, ofs);
            glEnable(GL_TEXTURE_2D);
            ofs += 2;
        }
    }

    if (prg->shader_id & SHADER_OPT_FOG) {
        // have fog, but fog colors are the same for every vertex
        // TODO: alpha ain't the same, maybe use glSecondaryColorPointer?
        // TODO: or pass start and end from gsSPFogPosition somehow or calculate them from z0, w0 and a0?
        // TODO: or alpha blend solid triangles on top, using the fog factor as alpha
        // glEnable(GL_FOG);
        // glFogi(GL_FOG_MODE, GL_LINEAR);
        // glFogfv(GL_FOG_COLOR, ofs);
        ofs += 4;
    }

    if (prg->num_inputs) {
        // have colors
        // TODO: more than one color (maybe glSecondaryColorPointer?)
        // HACK: if there's a texture and two colors, one of them is likely for speculars or some shit (see mario head)
        //       if there's two colors but no texture, the real color is likely the second one
        const int hack = (prg->num_inputs > 1) * (4 - (int)prg->texture_used[0]);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, cur_buf_stride, ofs + hack);
        ofs += 4 * prg->num_inputs;
    }

    if (prg->shader_id & SHADER_OPT_TEXTURE_EDGE) {
        // (horrible) alpha discard
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.3f);
    }

    // configure formulae
    switch (prg->mix) {
        case SH_MT_TEXTURE:
            glActiveTexture(GL_TEXTURE0);
            TEXENV_COMBINE_OFF();
            break;

        case SH_MT_TEXTURE_COLOR:
            texenv_set_texture_color(prg);
            break;

        case SH_MT_TEXTURE_TEXTURE:
            texenv_set_texture_texture(prg);
            break;

        default:
            break;
    }
}

static void gfx_opengl_unload_shader(struct ShaderProgram *old_prg) {
    if (cur_shader == old_prg || old_prg == NULL)
        cur_shader = NULL;

    glClientActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glClientActiveTexture(GL_TEXTURE1);
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE1);
    glDisable(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);

    glDisableClientState(GL_COLOR_ARRAY);
}

static void gfx_opengl_load_shader(struct ShaderProgram *new_prg) {
    cur_shader = new_prg;
    // gfx_opengl_apply_shader(cur_shader);
}

static struct ShaderProgram *gfx_opengl_create_and_load_new_shader(uint32_t shader_id) {
    uint8_t c[2][4];
    for (int i = 0; i < 4; i++) {
        c[0][i] = (shader_id >> (i * 3)) & 7;
        c[1][i] = (shader_id >> (12 + i * 3)) & 7;
    }

    bool used_textures[2] = {0, 0};
    int num_inputs = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (c[i][j] >= SHADER_INPUT_1 && c[i][j] <= SHADER_INPUT_4) {
                if (c[i][j] > num_inputs) {
                    num_inputs = c[i][j];
                }
            }
            if (c[i][j] == SHADER_TEXEL0 || c[i][j] == SHADER_TEXEL0A) {
                used_textures[0] = true;
            }
            if (c[i][j] == SHADER_TEXEL1) {
                used_textures[1] = true;
            }
        }
    }

    const bool color_alpha_same = (shader_id & 0xfff) == ((shader_id >> 12) & 0xfff);
    const bool do_multiply[2] = {c[0][1] == 0 && c[0][3] == 0, c[1][1] == 0 && c[1][3] == 0};
    const bool do_mix[2] = {c[0][1] == c[0][3], c[1][1] == c[1][3]};
    const bool do_single[2] = {c[0][2] == 0, c[1][2] == 0};

    struct ShaderProgram *prg = &shader_program_pool[shader_program_pool_size++];

    prg->shader_id = shader_id;
    prg->num_inputs = num_inputs;
    prg->texture_used[0] = used_textures[0];
    prg->texture_used[1] = used_textures[1];

    if (used_textures[0] && used_textures[1])
        prg->mix = SH_MT_TEXTURE_TEXTURE;
    else if (used_textures[0] && num_inputs)
        prg->mix = SH_MT_TEXTURE_COLOR;
    else if (used_textures[0])
        prg->mix = SH_MT_TEXTURE;
    else if (num_inputs > 1)
        prg->mix = SH_MT_COLOR_COLOR;
    else if (num_inputs)
        prg->mix = SH_MT_COLOR;

    if (do_single[0]) prg->mix_flags |= SH_MF_SINGLE;
    if (do_multiply[0]) prg->mix_flags |= SH_MF_MULTIPLY;
    if (do_mix[0]) prg->mix_flags |= SH_MF_MIX;

    if (!color_alpha_same && (shader_id & SHADER_OPT_ALPHA)) {
        prg->mix_flags |= SH_MF_OVERRIDE_ALPHA;
        if (do_single[1]) prg->mix_flags |= SH_MF_SINGLE_ALPHA;
        if (do_multiply[1]) prg->mix_flags |= SH_MF_MULTIPLY_ALPHA;
        if (do_mix[1]) prg->mix_flags |= SH_MF_MIX_ALPHA;
        if (c[1][3] < SHADER_TEXEL0) prg->mix_flags |= SH_MF_INPUT_ALPHA;
    }

    gfx_opengl_load_shader(prg);

    return prg;
}

static struct ShaderProgram *gfx_opengl_lookup_shader(uint32_t shader_id) {
    for (size_t i = 0; i < shader_program_pool_size; i++) {
        if (shader_program_pool[i].shader_id == shader_id) {
            return &shader_program_pool[i];
        }
    }
    return NULL;
}

static void gfx_opengl_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    *num_inputs = prg->num_inputs;
    used_textures[0] = prg->texture_used[0];
    used_textures[1] = prg->texture_used[1];
}

static GLuint gfx_opengl_new_texture(void) {
    GLuint ret;
    glGenTextures(1, &ret);
    return ret;
}

static void gfx_opengl_select_texture(int tile, GLuint texture_id) {
    glActiveTexture(GL_TEXTURE0 + tile);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

static void gfx_opengl_upload_texture(uint8_t *rgba32_buf, int width, int height) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba32_buf);
}

static uint32_t gfx_cm_to_opengl(uint32_t val) {
    if (val & G_TX_CLAMP)
        return GL_CLAMP_TO_EDGE;
    return (val & G_TX_MIRROR) ? GL_MIRRORED_REPEAT : GL_REPEAT;
}

static void gfx_opengl_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    const GLenum filter = linear_filter ? GL_LINEAR : GL_NEAREST;
    glActiveTexture(GL_TEXTURE0 + tile);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gfx_cm_to_opengl(cms));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gfx_cm_to_opengl(cmt));
}

static void gfx_opengl_set_depth_test(bool depth_test) {
    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

static void gfx_opengl_set_depth_mask(bool z_upd) {
    glDepthMask(z_upd ? GL_TRUE : GL_FALSE);
}

static void gfx_opengl_set_zmode_decal(bool zmode_decal) {
    if (zmode_decal) {
        glPolygonOffset(-2, -2);
        glEnable(GL_POLYGON_OFFSET_FILL);
    } else {
        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

static void gfx_opengl_set_viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

static void gfx_opengl_set_scissor(int x, int y, int width, int height) {
    glScissor(x, y, width, height);
}

static void gfx_opengl_set_use_alpha(bool use_alpha) {
    if (use_alpha) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

static void gfx_opengl_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    //printf("flushing %d tris\n", buf_vbo_num_tris);

    cur_buf = buf_vbo;
    cur_buf_size = buf_vbo_len * 4;
    cur_buf_num_tris = buf_vbo_num_tris;
    cur_buf_stride = cur_buf_size / (3 * cur_buf_num_tris);

    gfx_opengl_apply_shader(cur_shader);

    glDrawArrays(GL_TRIANGLES, 0, 3 * buf_vbo_num_tris);
}

static inline bool gl_check_ext(const char *name) {
    static const char *extstr = NULL;

    if (extstr == NULL)
        extstr = (const char *)glGetString(GL_EXTENSIONS);

    if (!strstr(extstr, name)) {
        fprintf(stderr, "Required GL extension not supported: %s\n", name);
        return false;
    }

    return true;
}

static inline bool gl_get_version(int *major, int *minor, bool *is_es) {
    const char *vstr = (const char *)glGetString(GL_VERSION);
    if (!vstr || !vstr[0]) return false;

    if (!strncmp(vstr, "OpenGL ES ", 10)) {
        vstr += 10;
        *is_es = true;
    } else if (!strncmp(vstr, "OpenGL ES-CM ", 13)) {
        vstr += 13;
        *is_es = true;
    }

    return (sscanf(vstr, "%d.%d", major, minor) == 2);
}

static void gfx_opengl_init(void) {
#if FOR_WINDOWS || defined(OSX_BUILD)
    glewInit();
#endif

    // check GL version
    int vmajor, vminor;
    bool is_es = false;
    gl_get_version(&vmajor, &vminor, &is_es);
    if (vmajor < 2 && vminor < 2 && !is_es) {
        fprintf(stderr, "OpenGL 1.2+ is required. Reported version: %s%d.%d\n", is_es ? "ES" : "", vmajor, vminor);
        abort();
    }

    // check extensions that we need
    bool supported =
        gl_check_ext("GL_ARB_multitexture") &&
        gl_check_ext("GL_ARB_texture_env_combine");

    if (!supported) abort();

    printf("GL_VERSION = %s\n", glGetString(GL_VERSION));
    printf("GL_EXTENSIONS =\n%s\n", glGetString(GL_EXTENSIONS));

    // these never change
    glEnableClientState(GL_VERTEX_ARRAY);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, c_white);
    TEXENV_COMBINE_OP(0, GL_SRC_COLOR, GL_SRC_ALPHA);
    TEXENV_COMBINE_OP(1, GL_SRC_COLOR, GL_SRC_ALPHA);
}

static void gfx_opengl_start_frame(void) {
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE); // Must be set to clear Z-buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

static void gfx_opengl_shutdown(void) {
}

struct GfxRenderingAPI gfx_opengl_api = {
    gfx_opengl_z_is_from_0_to_1,
    gfx_opengl_unload_shader,
    gfx_opengl_load_shader,
    gfx_opengl_create_and_load_new_shader,
    gfx_opengl_lookup_shader,
    gfx_opengl_shader_get_info,
    gfx_opengl_new_texture,
    gfx_opengl_select_texture,
    gfx_opengl_upload_texture,
    gfx_opengl_set_sampler_parameters,
    gfx_opengl_set_depth_test,
    gfx_opengl_set_depth_mask,
    gfx_opengl_set_zmode_decal,
    gfx_opengl_set_viewport,
    gfx_opengl_set_scissor,
    gfx_opengl_set_use_alpha,
    gfx_opengl_draw_triangles,
    gfx_opengl_init,
    gfx_opengl_start_frame,
    gfx_opengl_shutdown
};

#endif // LEGACY_GL
