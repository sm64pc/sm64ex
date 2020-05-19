#include <stdint.h>
#include <stdbool.h>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#ifdef __MINGW32__
#define FOR_WINDOWS 1
#else
#define FOR_WINDOWS 0
#endif

#if FOR_WINDOWS
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

#else
#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1

#ifdef OSX_BUILD
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles2.h>
#endif

#endif

#include "gfx_cc.h"
#include "gfx_rendering_api.h"

struct ShaderProgram {
    uint32_t shader_id;
    GLuint opengl_program_id;
    uint8_t num_inputs;
    bool used_textures[2];
    uint8_t num_floats;
    GLint attrib_locations[7];
    uint8_t attrib_sizes[7];
    uint8_t num_attribs;
};

static struct ShaderProgram shader_program_pool[64];
static uint8_t shader_program_pool_size;
static GLuint opengl_vbo;

static bool gfx_opengl_z_is_from_0_to_1(void) {
    return false;
}

static void gfx_opengl_vertex_array_set_attribs(struct ShaderProgram *prg) {
    size_t num_floats = prg->num_floats;
    size_t pos = 0;

    for (int i = 0; i < prg->num_attribs; i++) {
        glEnableVertexAttribArray(prg->attrib_locations[i]);
        glVertexAttribPointer(prg->attrib_locations[i], prg->attrib_sizes[i], GL_FLOAT, GL_FALSE, num_floats * sizeof(float), (void *)(pos * sizeof(float)));
        pos += prg->attrib_sizes[i];
    }
}

static void gfx_opengl_unload_shader(struct ShaderProgram *old_prg) {
    if (old_prg != NULL) {
        for (int i = 0; i < old_prg->num_attribs; i++) {
            glDisableVertexAttribArray(old_prg->attrib_locations[i]);
        }
    }
}

static void gfx_opengl_load_shader(struct ShaderProgram *new_prg) {
    glUseProgram(new_prg->opengl_program_id);
    gfx_opengl_vertex_array_set_attribs(new_prg);
}

static void append_str(char *buf, size_t *len, const char *str) {
    while (*str != '\0') buf[(*len)++] = *str++;
}

static void append_line(char *buf, size_t *len, const char *str) {
    while (*str != '\0') buf[(*len)++] = *str++;
    buf[(*len)++] = '\n';
}

static const char *shader_item_to_str(uint32_t item, bool with_alpha, bool only_alpha, bool inputs_have_alpha, bool hint_single_element) {
    if (!only_alpha) {
        switch (item) {
            case SHADER_0:
                return with_alpha ? "vec4(0.0, 0.0, 0.0, 0.0)" : "vec3(0.0, 0.0, 0.0)";
            case SHADER_INPUT_1:
                return with_alpha || !inputs_have_alpha ? "vInput1" : "vInput1.rgb";
            case SHADER_INPUT_2:
                return with_alpha || !inputs_have_alpha ? "vInput2" : "vInput2.rgb";
            case SHADER_INPUT_3:
                return with_alpha || !inputs_have_alpha ? "vInput3" : "vInput3.rgb";
            case SHADER_INPUT_4:
                return with_alpha || !inputs_have_alpha ? "vInput4" : "vInput4.rgb";
            case SHADER_TEXEL0:
                return with_alpha ? "texVal0" : "texVal0.rgb";
            case SHADER_TEXEL0A:
                return hint_single_element ? "texVal0.a" :
                    (with_alpha ? "vec4(texelVal0.a, texelVal0.a, texelVal0.a, texelVal0.a)" : "vec3(texelVal0.a, texelVal0.a, texelVal0.a)");
            case SHADER_TEXEL1:
                return with_alpha ? "texVal1" : "texVal1.rgb";
        }
    } else {
        switch (item) {
            case SHADER_0:
                return "0.0";
            case SHADER_INPUT_1:
                return "vInput1.a";
            case SHADER_INPUT_2:
                return "vInput2.a";
            case SHADER_INPUT_3:
                return "vInput3.a";
            case SHADER_INPUT_4:
                return "vInput4.a";
            case SHADER_TEXEL0:
                return "texVal0.a";
            case SHADER_TEXEL0A:
                return "texVal0.a";
            case SHADER_TEXEL1:
                return "texVal1.a";
        }
    }
}

static void append_formula(char *buf, size_t *len, uint8_t c[2][4], bool do_single, bool do_multiply, bool do_mix, bool with_alpha, bool only_alpha, bool opt_alpha) {
    if (do_single) {
        append_str(buf, len, shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    } else if (do_multiply) {
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " * ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
    } else if (do_mix) {
        append_str(buf, len, "mix(");
        append_str(buf, len, shader_item_to_str(c[only_alpha][1], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ", ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ", ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
        append_str(buf, len, ")");
    } else {
        append_str(buf, len, "(");
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " - ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][1], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ") * ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
        append_str(buf, len, " + ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    }
}

static struct ShaderProgram *gfx_opengl_create_and_load_new_shader(uint32_t shader_id) {
    uint8_t c[2][4];
    for (int i = 0; i < 4; i++) {
        c[0][i] = (shader_id >> (i * 3)) & 7;
        c[1][i] = (shader_id >> (12 + i * 3)) & 7;
    }
    bool opt_alpha = (shader_id & SHADER_OPT_ALPHA) != 0;
    bool opt_fog = (shader_id & SHADER_OPT_FOG) != 0;
    bool opt_texture_edge = (shader_id & SHADER_OPT_TEXTURE_EDGE) != 0;
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
    bool do_single[2] = {c[0][2] == 0, c[1][2] == 0};
    bool do_multiply[2] = {c[0][1] == 0 && c[0][3] == 0, c[1][1] == 0 && c[1][3] == 0};
    bool do_mix[2] = {c[0][1] == c[0][3], c[1][1] == c[1][3]};
    bool color_alpha_same = (shader_id & 0xfff) == ((shader_id >> 12) & 0xfff);

    char vs_buf[1024];
    char fs_buf[1024];
    size_t vs_len = 0;
    size_t fs_len = 0;
    size_t num_floats = 4;

    // Vertex shader
#ifdef OSX_BUILD
    append_line(vs_buf, &vs_len, "");
#else
    append_line(vs_buf, &vs_len, "#version 100");
#endif
    append_line(vs_buf, &vs_len, "attribute vec4 aVtxPos;");
    if (used_textures[0] || used_textures[1]) {
        append_line(vs_buf, &vs_len, "attribute vec2 aTexCoord;");
        append_line(vs_buf, &vs_len, "varying vec2 vTexCoord;");
        num_floats += 2;
    }
    if (opt_fog) {
        append_line(vs_buf, &vs_len, "attribute vec4 aFog;");
        append_line(vs_buf, &vs_len, "varying vec4 vFog;");
        num_floats += 4;
    }
    for (int i = 0; i < num_inputs; i++) {
        vs_len += sprintf(vs_buf + vs_len, "attribute vec%d aInput%d;\n", opt_alpha ? 4 : 3, i + 1);
        vs_len += sprintf(vs_buf + vs_len, "varying vec%d vInput%d;\n", opt_alpha ? 4 : 3, i + 1);
        num_floats += opt_alpha ? 4 : 3;
    }
    append_line(vs_buf, &vs_len, "void main() {");
    if (used_textures[0] || used_textures[1]) {
        append_line(vs_buf, &vs_len, "vTexCoord = aTexCoord;");
    }
    if (opt_fog) {
        append_line(vs_buf, &vs_len, "vFog = aFog;");
    }
    for (int i = 0; i < num_inputs; i++) {
        vs_len += sprintf(vs_buf + vs_len, "vInput%d = aInput%d;\n", i + 1, i + 1);
    }
    append_line(vs_buf, &vs_len, "gl_Position = aVtxPos;");
    append_line(vs_buf, &vs_len, "}");

    // Fragment shader
#ifdef OSX_BUILD
    append_line(fs_buf, &fs_len, "");
#else
    append_line(fs_buf, &fs_len, "#version 100");
    append_line(fs_buf, &fs_len, "precision mediump float;");
#endif

    if (used_textures[0] || used_textures[1]) {
        append_line(fs_buf, &fs_len, "varying vec2 vTexCoord;");
    }
    if (opt_fog) {
        append_line(fs_buf, &fs_len, "varying vec4 vFog;");
    }
    for (int i = 0; i < num_inputs; i++) {
        fs_len += sprintf(fs_buf + fs_len, "varying vec%d vInput%d;\n", opt_alpha ? 4 : 3, i + 1);
    }
    if (used_textures[0]) {
        append_line(fs_buf, &fs_len, "uniform sampler2D uTex0;");
    }
    if (used_textures[1]) {
        append_line(fs_buf, &fs_len, "uniform sampler2D uTex1;");
    }
    append_line(fs_buf, &fs_len, "void main() {");
    
    if (used_textures[0]) {
        append_line(fs_buf, &fs_len, "vec4 texVal0 = texture2D(uTex0, vTexCoord);");
    }
    if (used_textures[1]) {
        append_line(fs_buf, &fs_len, "vec4 texVal1 = texture2D(uTex1, vTexCoord);");
    }
    
    append_str(fs_buf, &fs_len, opt_alpha ? "vec4 texel = " : "vec3 texel = ");
    if (!color_alpha_same && opt_alpha) {
        append_str(fs_buf, &fs_len, "vec4(");
        append_formula(fs_buf, &fs_len, c, do_single[0], do_multiply[0], do_mix[0], false, false, true);
        append_str(fs_buf, &fs_len, ", ");
        append_formula(fs_buf, &fs_len, c, do_single[1], do_multiply[1], do_mix[1], true, true, true);
        append_str(fs_buf, &fs_len, ")");
    } else {
        append_formula(fs_buf, &fs_len, c, do_single[0], do_multiply[0], do_mix[0], opt_alpha, false, opt_alpha);
    }
    append_line(fs_buf, &fs_len, ";");
    
    if (opt_texture_edge && opt_alpha) {
        append_line(fs_buf, &fs_len, "if (texel.a > 0.3) texel.a = 1.0; else discard;");
    }
    // TODO discard if alpha is 0?
    if (opt_fog) {
        if (opt_alpha) {
            append_line(fs_buf, &fs_len, "texel = vec4(mix(texel.rgb, vFog.rgb, vFog.a), texel.a);");
        } else {
            append_line(fs_buf, &fs_len, "texel = mix(texel, vFog.rgb, vFog.a);");
        }
    }
    
    if (opt_alpha) {
        append_line(fs_buf, &fs_len, "gl_FragColor = texel;");
    } else {
        append_line(fs_buf, &fs_len, "gl_FragColor = vec4(texel, 1.0);");
    }
    append_line(fs_buf, &fs_len, "}");
    
    vs_buf[vs_len] = '\0';
    fs_buf[fs_len] = '\0';
    
    /*puts("Vertex shader:");
    puts(vs_buf);
    puts("Fragment shader:");
    puts(fs_buf);
    puts("End");*/
    
    const GLchar *sources[2] = {vs_buf, fs_buf};
    const GLint lengths[2] = {vs_len, fs_len};
    GLint success;
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &sources[0], &lengths[0]);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint max_length = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &max_length);
        char error_log[1024];
        fprintf(stderr, "Vertex shader compilation failed\n");
        glGetShaderInfoLog(vertex_shader, max_length, &max_length, &error_log[0]);
        fprintf(stderr, "%s\n", &error_log[0]);
        abort();
    }
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &sources[1], &lengths[1]);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint max_length = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);
        char error_log[1024];
        fprintf(stderr, "Fragment shader compilation failed\n");
        glGetShaderInfoLog(fragment_shader, max_length, &max_length, &error_log[0]);
        fprintf(stderr, "%s\n", &error_log[0]);
        abort();
    }
    
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    size_t cnt = 0;
    
    struct ShaderProgram *prg = &shader_program_pool[shader_program_pool_size++];
    prg->attrib_locations[cnt] = glGetAttribLocation(shader_program, "aVtxPos");
    prg->attrib_sizes[cnt] = 4;
    ++cnt;
    
    if (used_textures[0] || used_textures[1]) {
        prg->attrib_locations[cnt] = glGetAttribLocation(shader_program, "aTexCoord");
        prg->attrib_sizes[cnt] = 2;
        ++cnt;
    }
    
    if (opt_fog) {
        prg->attrib_locations[cnt] = glGetAttribLocation(shader_program, "aFog");
        prg->attrib_sizes[cnt] = 4;
        ++cnt;
    }
    
    for (int i = 0; i < num_inputs; i++) {
        char name[16];
        sprintf(name, "aInput%d", i + 1);
        prg->attrib_locations[cnt] = glGetAttribLocation(shader_program, name);
        prg->attrib_sizes[cnt] = opt_alpha ? 4 : 3;
        ++cnt;
    }
    
    prg->shader_id = shader_id;
    prg->opengl_program_id = shader_program;
    prg->num_inputs = num_inputs;
    prg->used_textures[0] = used_textures[0];
    prg->used_textures[1] = used_textures[1];
    prg->num_floats = num_floats;
    prg->num_attribs = cnt;
    
    gfx_opengl_load_shader(prg);
    
    if (used_textures[0]) {
        GLint sampler_attrib = glGetUniformLocation(shader_program, "uTex0");
        glUniform1i(sampler_attrib, 0);
    }
    if (used_textures[1]) {
        GLint sampler_attrib = glGetUniformLocation(shader_program, "uTex1");
        glUniform1i(sampler_attrib, 1);
    }
    
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
    used_textures[0] = prg->used_textures[0];
    used_textures[1] = prg->used_textures[1];
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
    if (val & G_TX_CLAMP) {
        return GL_CLAMP_TO_EDGE;
    }
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buf_vbo_len, buf_vbo, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3 * buf_vbo_num_tris);
}

static void gfx_opengl_init(void) {
#if FOR_WINDOWS
    glewInit();
#endif

#ifdef OSX_BUILD
    glewInit();
#endif

    glGenBuffers(1, &opengl_vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, opengl_vbo);
    
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
