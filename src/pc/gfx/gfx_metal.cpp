#ifdef RAPI_METAL
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <chrono>
#include <format>
#include <iomanip>
#include <vector>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <PR/gbi.h>

#include "gfx_rendering_api.h"
#include "gfx_cc.h"
#include "../configfile.h"

#define DECLARE_GFX_SDL_FUNCTIONS
#include "gfx_sdl.h"

void log_error(const char *fmt, ...) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_struct = *std::localtime(&t);

    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;

    std::cout << "[" << std::put_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
    std::cout << "." << std::setfill('0') << std::setw(6) << microseconds << "] ";

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    fputc('\n', stderr);
    va_end(args);
}

struct TextureDataMetal {
    MTL::Texture *texture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sampler_parameters = 0;
};

struct ShaderProgramMetal {
    uint32_t shader_id;
    bool used_noise;
    bool used_textures[2];
    uint8_t num_inputs;
    MTL::RenderPipelineState *pipeline = nullptr;
};

struct MetalUniforms {
    uint32_t noise_frame;
    float noise_scale_x;
    float noise_scale_y;
    float _pad;
};

struct {
    // long life stuff
    CA::MetalLayer *layer = nullptr;
    MTL::Device *device = nullptr;
    MTL::CommandQueue *queue = nullptr;

    MTL::Texture *depth_texture = nullptr;

    struct ShaderProgramMetal shader_program_pool[64];
    uint8_t shader_program_pool_size = 0;

    std::vector<TextureDataMetal> textures;
    std::vector<MTL::SamplerState *> samplers;

    MTL::Buffer *dynamic_vertex_buffer = nullptr;
    size_t dynamic_offset = 0;

    // state stuff
    bool new_encoder = false;
    bool viewport_did_change = false;
    bool scissor_did_change = false;
    MTL::Viewport viewport;
    MTL::ScissorRect scissor;
    ShaderProgramMetal *active_shader = nullptr;
    uint32_t current_texture_ids[2] = {};
    int current_tile = 0;
    MTL::Buffer *uniforms_buffer = nullptr;
    uint32_t current_width = 0;
    uint32_t current_height = 0;

    // previous state stuff
    ShaderProgramMetal *last_shader = nullptr;
    bool last_depth_mask = false;
    bool last_depth_test = false;
    bool last_zmode_decal = false;
    // force these to be different from current_texture_ids so the comparison passes on the first run
    uint32_t previous_texture_ids[2] = {0xffffffff, 0xffffffff};

    // depth stuff
    bool depth_mask = false;
    bool depth_test = false;
    bool zmode_decal = false;
    bool needs_resize_depth = false;
    MTL::DepthStencilState *depth_states[2] = {};
    MTL::DepthStencilState *depth_state_disabled = nullptr;

    // per frame stuff
    CA::MetalDrawable *current_drawable = nullptr;
    MTL::CommandBuffer *current_cmd_buffer = nullptr;
    MTL::RenderCommandEncoder *current_encoder = nullptr;
    MTL::RenderPassDescriptor *current_pass_desc = nullptr;
    NS::AutoreleasePool *autorelease_pool = nullptr;
} mtl_state;


static void create_depth_texture(int w, int h)
{
    if (mtl_state.depth_texture) {
        mtl_state.depth_texture->release();
        mtl_state.depth_texture = nullptr;
    }
    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatDepth32Float,
        w,
        h,
        false
    );
    desc->setUsage(MTL::TextureUsageRenderTarget);
    desc->setStorageMode(MTL::StorageModePrivate);
    mtl_state.depth_texture = mtl_state.device->newTexture(desc);
}

static const char *shader_item_to_str(uint32_t item, bool with_alpha, bool only_alpha, bool inputs_have_alpha, bool hint_single_element) {
    if (!only_alpha) {
        switch (item) {
            default:
            case SHADER_0:
                return with_alpha ? "float4(0.0, 0.0, 0.0, 0.0)" : "float3(0.0, 0.0, 0.0)";
            case SHADER_INPUT_1:
                return with_alpha || !inputs_have_alpha ? "in.input1" : "in.input1.rgb";
            case SHADER_INPUT_2:
                return with_alpha || !inputs_have_alpha ? "in.input2" : "in.input2.rgb";
            case SHADER_INPUT_3:
                return with_alpha || !inputs_have_alpha ? "in.input3" : "in.input3.rgb";
            case SHADER_INPUT_4:
                return with_alpha || !inputs_have_alpha ? "in.input4" : "in.input4.rgb";
            case SHADER_TEXEL0:
                return with_alpha ? "texVal0" : "texVal0.rgb";
            case SHADER_TEXEL0A:
                return hint_single_element ? "texVal0.a" : (with_alpha ? "float4(texVal0.a, texVal0.a, texVal0.a, texVal0.a)" : "float3(texVal0.a, texVal0.a, texVal0.a)");
            case SHADER_TEXEL1:
                return with_alpha ? "texVal1" : "texVal1.rgb";
        }
    } else {
        switch (item) {
            default:
            case SHADER_0:
                return "0.0";
            case SHADER_INPUT_1:
                return "in.input1.a";
            case SHADER_INPUT_2:
                return "in.input2.a";
            case SHADER_INPUT_3:
                return "in.input3.a";
            case SHADER_INPUT_4:
                return "in.input4.a";
            case SHADER_TEXEL0:
                return "texVal0.a";
            case SHADER_TEXEL0A:
                return "texVal0.a";
            case SHADER_TEXEL1:
                return "texVal1.a";
        }
    }
}


static std::string generate_formula(const uint8_t c[2][4],
                                    bool do_single, bool do_multiply, bool do_mix,
                                    bool with_alpha, bool only_alpha, bool inputs_have_alpha) {
    std::string expr;

    auto item = [&](int index, bool hint_single=false) {
        return shader_item_to_str(c[only_alpha][index], with_alpha, only_alpha, inputs_have_alpha, hint_single);
    };

    if (do_single) {
        expr += item(3);
    } else if (do_multiply) {
        expr += item(0);
        expr += " * ";
        expr += item(2,true);
    } else if (do_mix) {
        expr += "mix(";
        expr += item(1);
        expr += ", ";
        expr += item(0);
        expr += ", ";
        expr += item(2,true);
        expr += ")";
    } else {
        expr += "(";
        expr += item(0);
        expr += " - ";
        expr += item(1);
        expr += ") * ";
        expr += item(2,true);
        expr += " + ";
        expr += item(3);
    }

    return expr;
}

static bool gfx_metal_z_is_from_0_to_1(void) {
    return true;
}

static void gfx_metal_unload_shader(ShaderProgram *old_prg) {
    // nothing to do
}

static void gfx_metal_load_shader(ShaderProgram *new_prg) {
    mtl_state.last_shader = mtl_state.active_shader;
    mtl_state.active_shader = reinterpret_cast<ShaderProgramMetal *>(new_prg);
}

static ShaderProgram *gfx_metal_create_and_load_new_shader(uint32_t shader_id) {

    CCFeatures ccf;
    gfx_cc_get_features(shader_id, &ccf);

    auto vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();

    std::string vertex_shader_source = "";
    vertex_shader_source += "#include <metal_stdlib>\nusing namespace metal;\n";

    std::string input_struct_source = "struct VertexIn {\n";
    std::string output_struct_source = "struct VertexOut {\n";

    input_struct_source += "    float4 pos [[attribute(0)]];\n";
    output_struct_source += "    float4 pos [[position]];\n";
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);


    int input_idx = 1;
    int shader_input_idx = 1;
    int shader_output_idx = 1;
    int num_floats = 4; // position added

    if (ccf.used_textures[0] || ccf.used_textures[1]) {
        input_struct_source += std::format("    float2 uv [[attribute({})]];\n",
            shader_input_idx);
        output_struct_source += std::format("    float2 uv;\n");

        vertexDescriptor->attributes()->object(shader_input_idx)->setFormat(MTL::VertexFormatFloat2);
        vertexDescriptor->attributes()->object(shader_input_idx)->setOffset(num_floats * sizeof(float));
        vertexDescriptor->attributes()->object(shader_input_idx)->setBufferIndex(0);

        shader_output_idx++;
        num_floats += 2;
        shader_input_idx++;
    }

    if (ccf.opt_fog) {
        input_struct_source += std::format("    float4 fog [[attribute({})]];\n",
            shader_input_idx);
        output_struct_source += std::format("    float4 fog;\n");
        vertexDescriptor->attributes()->object(shader_input_idx)->setFormat(MTL::VertexFormatFloat4);
        vertexDescriptor->attributes()->object(shader_input_idx)->setOffset(num_floats * sizeof(float));
        vertexDescriptor->attributes()->object(shader_input_idx)->setBufferIndex(0);
        shader_input_idx++;
        shader_output_idx++;
        num_floats += 4;
    }

    for (int i = 0; i < ccf.num_inputs; i++) {
        int input_n_floats = ccf.opt_alpha ? 4 : 3;
        input_struct_source += std::format("    float{} input{} [[attribute({})]];\n", input_n_floats, input_idx, shader_input_idx);
        output_struct_source += std::format("    float{} input{};\n", input_n_floats, input_idx);
        vertexDescriptor->attributes()->object(shader_input_idx)->setFormat(input_n_floats == 4 ? MTL::VertexFormatFloat4 : MTL::VertexFormatFloat3);
        vertexDescriptor->attributes()->object(shader_input_idx)->setOffset(num_floats * sizeof(float));
        vertexDescriptor->attributes()->object(shader_input_idx)->setBufferIndex(0);
        shader_input_idx++;
        shader_output_idx++;
        input_idx++;
        num_floats += input_n_floats;
    }
    input_struct_source += "};\n\n";
    output_struct_source += "};\n\n";
    vertex_shader_source += input_struct_source;
    vertex_shader_source += output_struct_source;

    vertex_shader_source += R"(vertex VertexOut vertex_main(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.pos = in.pos;
)";

    if (ccf.used_textures[0] || ccf.used_textures[1]) {
        vertex_shader_source += "    out.uv = in.uv;\n";
    }
    if (ccf.opt_fog) {
        vertex_shader_source += "    out.fog = in.fog;\n";
    }
    for (int i = 0; i < ccf.num_inputs; i++) {
        vertex_shader_source += std::format("    out.input{} = in.input{};\n", input_idx - i - 1, input_idx - i - 1);
    }

    vertex_shader_source += "    return out;\n}\n\n";

    std::string fs;

    if (ccf.opt_noise) {
        fs += "struct MetalUniforms {\n";
        fs += "    uint noise_frame;\n";
        fs += "    float noise_scale_x;\n";
        fs += "    float noise_scale_y;\n";
        fs += "    float _pad;\n";
        fs += "};\n\n";
    }


    // Sampling helper
    if (ccf.used_textures[0] || ccf.used_textures[1]) {
        fs += "float4 sampleTex(texture2d<float> tex, sampler samp, float2 uv, float2 texSize) {\n";
        if (configFiltering == 2) {
            fs += "    float2 offset = fract(uv * texSize - float2(0.5));\n";
            fs += "    offset -= step(1.0, offset.x + offset.y);\n";
            fs += "    float2 invSize = 1.0 / texSize;\n";
            fs += "    float4 c0 = tex.sample(samp, uv - offset * invSize);\n";
            fs += "    float4 c1 = tex.sample(samp, uv - float2(offset.x - sign(offset.x), offset.y) * invSize);\n";
            fs += "    float4 c2 = tex.sample(samp, uv - float2(offset.x, offset.y - sign(offset.y)) * invSize);\n";
            fs += "    return c0 + abs(offset.x)*(c1-c0) + abs(offset.y)*(c2-c0);\n";
        } else {
            fs += "    return tex.sample(samp, uv);\n";
        }
        fs += "}\n\n";
    }

    // Random function
    if (ccf.opt_alpha && ccf.opt_noise) {
        fs += "float random(float3 value) {\n";
        fs += "    float r = dot(sin(value), float3(12.9898, 78.233, 37.719));\n";
        fs += "    return fract(sin(r) * 143758.5453);\n";
        fs += "}\n\n";
    }

    // Fragment function
    fs += "fragment float4 fragment_main(\n";
    fs += "    VertexOut in [[stage_in]]";
    if (ccf.used_textures[0] || ccf.used_textures[1]) {
        fs += ",\n";
    }
    if (ccf.used_textures[0]) {
        fs += "    texture2d<float> uTex0 [[texture(0)]], sampler samp0 [[sampler(0)]]";
        if (ccf.used_textures[1]) fs+= ',';
        fs+= '\n';
     }
    if (ccf.used_textures[1]) fs += "    texture2d<float> uTex1 [[texture(1)]], sampler samp1 [[sampler(1)]]\n";
    if (ccf.opt_noise) fs += ",\n    constant MetalUniforms &uniforms [[buffer(0)]]";

    fs += ") {\n";

    // Sample textures
    if (ccf.used_textures[0]) fs += "    float4 texVal0 = sampleTex(uTex0, samp0, in.uv, float2(uTex0.get_width(), uTex0.get_height()));\n";
    if (ccf.used_textures[1]) fs += "    float4 texVal1 = sampleTex(uTex1, samp1, in.uv, float2(uTex1.get_width(), uTex1.get_height()));\n";


    fs += ccf.opt_alpha ? "    float4" : "    float3";
    fs += " texel = ";
    if (!ccf.color_alpha_same && ccf.opt_alpha) {
        fs += "float4(";
        fs += generate_formula(ccf.c, ccf.do_single[0], ccf.do_multiply[0], ccf.do_mix[0], false, false, true);
        fs += ",";
        fs += generate_formula(ccf.c, ccf.do_single[1], ccf.do_multiply[1], ccf.do_mix[1], true, true, true);
        fs += ")";
    } else {
        fs += generate_formula(ccf.c, ccf.do_single[0], ccf.do_multiply[0], ccf.do_mix[0], ccf.opt_alpha, false, ccf.opt_alpha);
    }
    fs += ";\n";


    // Edge alpha
    if (ccf.opt_texture_edge && ccf.opt_alpha)
        fs += "    if (texel.a <= 0.3) discard_fragment(); else texel.a = 1.0;\n";

    // Fog
    if (ccf.opt_fog) {
        if (ccf.opt_alpha) fs += "    texel.rgb = mix(texel.rgb, in.fog.rgb, in.fog.a);\n";
        else fs += "    texel = mix(texel, in.fog.rgb, in.fog.a);\n";
    }

    // Noise alpha
    if (ccf.opt_alpha && ccf.opt_noise) {
        fs += "    float2 coords = floor(in.pos.xy / float2(uniforms.noise_scale_x, uniforms.noise_scale_y));";
        fs += "    texel.a *= round(random(float3(coords, float(uniforms.noise_frame))));\n";
    }

    // Return
    fs += ccf.opt_alpha ? "    return texel;\n" : "    return float4(texel, 1.0);\n";

    fs += "}\n";

    std::string shader_combined_source = vertex_shader_source + fs;
    NS::Error *error = nullptr;
    auto library = mtl_state.device->newLibrary(
        NS::String::string(shader_combined_source.c_str(), NS::StringEncoding::UTF8StringEncoding), nullptr, &error);

    if (!library) {
        std::cout << "Library compilation failed: " << error->localizedDescription()->utf8String() << std::endl;
        exit(-1);
    }

    vertexDescriptor->layouts()->object(0)->setStride(num_floats * sizeof(float));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    ShaderProgramMetal *prg = &mtl_state.shader_program_pool[mtl_state.shader_program_pool_size++];
    prg->shader_id = shader_id;
    prg->used_noise = ccf.opt_noise;
    prg->num_inputs = ccf.num_inputs;
    prg->used_textures[0] = ccf.used_textures[0];
    prg->used_textures[1] = ccf.used_textures[1];

    auto vertexFunc = library->newFunction( NS::String::string("vertex_main", NS::UTF8StringEncoding));
    auto fragmentFunc = library->newFunction( NS::String::string("fragment_main", NS::UTF8StringEncoding));

    // create pipeline
    auto pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setVertexFunction(vertexFunc);
    pipelineDesc->setFragmentFunction(fragmentFunc);
    pipelineDesc->setVertexDescriptor(vertexDescriptor);
    pipelineDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);

    auto colorAttachment = pipelineDesc->colorAttachments()->object(0);
    colorAttachment->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    if (ccf.opt_alpha) {
        colorAttachment->setBlendingEnabled(true);
        colorAttachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        colorAttachment->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        colorAttachment->setRgbBlendOperation(MTL::BlendOperationAdd);
        colorAttachment->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
        colorAttachment->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        colorAttachment->setAlphaBlendOperation(MTL::BlendOperationAdd);
        colorAttachment->setWriteMask(MTL::ColorWriteMaskAll);
    } else {
        colorAttachment->setBlendingEnabled(false);
        colorAttachment->setWriteMask(MTL::ColorWriteMaskAll);
    }

    auto pipelineState = mtl_state.device->newRenderPipelineState(pipelineDesc, &error);
    if (!pipelineState) {
        log_error("failed to make pipeline. %s\n", error->localizedDescription()->utf8String());
    }
    pipelineDesc->release();

    prg->pipeline = pipelineState;

    gfx_metal_load_shader(reinterpret_cast<ShaderProgram *>(prg));
    return reinterpret_cast<ShaderProgram *>(prg);
}

static struct ShaderProgram *gfx_metal_lookup_shader(uint32_t shader_id) {
    for (size_t i = 0; i < mtl_state.shader_program_pool_size; i++) {
        if (mtl_state.shader_program_pool[i].shader_id == shader_id) {
            return reinterpret_cast<ShaderProgram *>(&mtl_state.shader_program_pool[i]);
        }
    }
    return nullptr;
}

static void gfx_metal_shader_get_info(ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    ShaderProgramMetal *mtl_prg = reinterpret_cast<ShaderProgramMetal *>(prg);

    *num_inputs = mtl_prg->num_inputs;
    used_textures[0] = mtl_prg->used_textures[0];
    used_textures[1] = mtl_prg->used_textures[1];
}

static uint32_t gfx_metal_new_texture(void) {
    mtl_state.textures.resize(mtl_state.textures.size() + 1);
    return (uint32_t)(mtl_state.textures.size() - 1);
}

static void gfx_metal_select_texture(int tile, uint32_t texture_id) {
    mtl_state.current_tile = tile;
    mtl_state.previous_texture_ids[tile] = mtl_state.current_texture_ids[tile];
    mtl_state.current_texture_ids[tile] = texture_id;
}


static void gfx_metal_upload_texture(const uint8_t *rgba32_buf,
                                     int width,
                                     int height)
{
    uint32_t texture_id = mtl_state.current_texture_ids[mtl_state.current_tile];

    TextureDataMetal &td = mtl_state.textures[texture_id];

    // Release old texture if replacing
    if (td.texture) {
        td.texture->release();
        td.texture = nullptr;
    }

    // Describe texture
    MTL::TextureDescriptor *desc =
        MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA8Unorm,
            width,
            height,
            false
        );

    desc->setUsage(MTL::TextureUsageShaderRead);
    desc->setStorageMode(MTL::StorageModeShared);

    td.texture = mtl_state.device->newTexture(desc);

    td.width = width;
    td.height = height;

    // Upload pixel data
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);

    td.texture->replaceRegion(region, 0, rgba32_buf, width * 4);
}

static int gfx_cm_to_index(uint32_t val) {
    if (val & G_TX_CLAMP) {
        return 2;
    }
    return (val & G_TX_MIRROR) ? 1 : 0;
}

static void gfx_metal_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    mtl_state.textures[mtl_state.current_texture_ids[tile]].sampler_parameters =
        linear_filter * 9 + gfx_cm_to_index(cms) * 3 + gfx_cm_to_index(cmt);
}

static void gfx_metal_set_depth_test(bool depth_test) {
    mtl_state.last_depth_test = mtl_state.depth_test;
    mtl_state.depth_test = depth_test;
}

static void gfx_metal_set_depth_mask(bool z_upd) {
    mtl_state.last_depth_mask = mtl_state.depth_mask;
    mtl_state.depth_mask = z_upd;
}

static void gfx_metal_set_zmode_decal(bool zmode_decal) {
    mtl_state.last_zmode_decal = mtl_state.zmode_decal;
    mtl_state.zmode_decal = zmode_decal;
}

static void gfx_metal_set_viewport(int x, int y, int width, int height) {
    MTL::Viewport viewport{
        static_cast<double>(x),
        static_cast<double>(y),
        static_cast<double>(width),
        static_cast<double>(height),
        0.0,
        1.0
    };
    mtl_state.viewport = viewport;
    mtl_state.viewport_did_change = true;
    mtl_state.needs_resize_depth = true;
    mtl_state.current_width = width;
    mtl_state.current_height = height;
    mtl_state.layer->setDrawableSize(CGSizeMake(width, height));
}

static void gfx_metal_set_scissor(int x, int y, int width, int height) {
    MTL::ScissorRect scissor{
        static_cast<NS::UInteger>(x),
        static_cast<NS::UInteger>(mtl_state.current_height - y - height),
        static_cast<NS::UInteger>(width),
        static_cast<NS::UInteger>(height)
    };
    mtl_state.scissor = scissor;
    mtl_state.scissor_did_change = true;
}

static void gfx_metal_set_use_alpha(bool use_alpha) {
    // part of the state from shader info i think
}

static void gfx_metal_draw_triangles(float buf_vbo[],
                                     size_t buf_vbo_len,
                                     size_t buf_vbo_num_tris)
{
    // do nothing if we failed to get drawable
    if (!mtl_state.current_drawable) {
        return;
    }

    if (mtl_state.viewport_did_change || mtl_state.new_encoder) {
        mtl_state.current_encoder->setViewport(mtl_state.viewport);
        mtl_state.viewport_did_change = false;
    }

    if (mtl_state.scissor_did_change || mtl_state.new_encoder) {
        mtl_state.current_encoder->setScissorRect(mtl_state.scissor);
        mtl_state.scissor_did_change = false;
    }

    if (mtl_state.active_shader != mtl_state.last_shader || mtl_state.new_encoder) {
        mtl_state.current_encoder->setRenderPipelineState(mtl_state.active_shader->pipeline);
        mtl_state.last_shader = mtl_state.active_shader;
    }
    if (mtl_state.zmode_decal != mtl_state.last_zmode_decal || mtl_state.new_encoder) {
        if (mtl_state.zmode_decal) {
            float slopeScale = -2.0f;
            float clamp = 0.0f;
            float constantBias = 0.0f;
            mtl_state.current_encoder->setDepthBias(constantBias, slopeScale, clamp);
        } else {
            mtl_state.current_encoder->setDepthBias(0, 0, 0);
        }
    }

    if ((mtl_state.depth_test != mtl_state.last_depth_test ||
    	mtl_state.depth_mask != mtl_state.last_depth_mask) || 
        mtl_state.new_encoder) {
        if (mtl_state.depth_test) {
            mtl_state.current_encoder->setDepthStencilState(mtl_state.depth_states[mtl_state.depth_mask ? 1 : 0]);
        } else {
            mtl_state.current_encoder->setDepthStencilState(mtl_state.depth_state_disabled);
        }
    }

    if (mtl_state.active_shader->used_textures[0] &&
        (mtl_state.previous_texture_ids[0] != mtl_state.current_texture_ids[0] || mtl_state.new_encoder)) {
        uint32_t tex_id1 = mtl_state.current_texture_ids[0];
        TextureDataMetal &td = mtl_state.textures[tex_id1];
        int index = td.sampler_parameters;

        mtl_state.current_encoder->setFragmentTexture(td.texture, 0);
        mtl_state.current_encoder->setFragmentSamplerState(mtl_state.samplers[index], 0);
    }

    if (mtl_state.active_shader->used_textures[1] &&
        (mtl_state.previous_texture_ids[1] != mtl_state.current_texture_ids[1] || mtl_state.new_encoder)) {
        uint32_t tex_id1 = mtl_state.current_texture_ids[1];
        TextureDataMetal &td = mtl_state.textures[tex_id1];
        int index = td.sampler_parameters;

        mtl_state.current_encoder->setFragmentTexture(td.texture, 1);
        mtl_state.current_encoder->setFragmentSamplerState(mtl_state.samplers[index], 1);
    }

    if (mtl_state.active_shader->used_noise) {
        mtl_state.current_encoder->setFragmentBuffer(mtl_state.uniforms_buffer, 0, 0);
    }

    size_t size = buf_vbo_len * sizeof(float);

    // Align to 256 bytes (required by Metal)
    size_t aligned = (size + 255) & ~255;

    void* dst = (uint8_t*)mtl_state.dynamic_vertex_buffer->contents()
                + mtl_state.dynamic_offset;

    memcpy(dst, buf_vbo, size);

    mtl_state.current_encoder->setVertexBufferOffset(mtl_state.dynamic_offset, 0);

    mtl_state.current_encoder->drawPrimitives(
        MTL::PrimitiveTypeTriangle,
        static_cast<NS::UInteger>(0),
        buf_vbo_num_tris * 3
    );

    mtl_state.dynamic_offset += aligned;
    mtl_state.new_encoder = false;
}


static void gfx_metal_init(void) {
    mtl_state.layer = static_cast<CA::MetalLayer *>(gfx_sdl_get_layer());
    mtl_state.device = MTL::CreateSystemDefaultDevice();

    //printf("[metal] using device %s\n", mtl_state.device->name()->utf8String());
    mtl_state.layer->setDevice(mtl_state.device);

    mtl_state.queue = mtl_state.device->newCommandQueue();

    mtl_state.dynamic_vertex_buffer = mtl_state.device->newBuffer(
        1 * 1024 * 1024, // 1MiB
        MTL::ResourceStorageModeShared
    );

    mtl_state.dynamic_offset = 0;

    mtl_state.uniforms_buffer = mtl_state.device->newBuffer(sizeof(MetalUniforms), MTL::ResourceStorageModeShared);


    auto sz = mtl_state.layer->drawableSize();

    NS::UInteger width = sz.width;
    NS::UInteger height = sz.height;

    create_depth_texture(width, height);

    auto d = MTL::DepthStencilDescriptor::alloc()->init();
    d->setDepthCompareFunction(MTL::CompareFunctionAlways);
    d->setDepthWriteEnabled(false);
    mtl_state.depth_state_disabled = mtl_state.device->newDepthStencilState(d);
    d->release();

    d = MTL::DepthStencilDescriptor::alloc()->init();
    d->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    d->setDepthWriteEnabled(false);
    mtl_state.depth_states[0] = mtl_state.device->newDepthStencilState(d);
    d->release();

    d = MTL::DepthStencilDescriptor::alloc()->init();
    d->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    d->setDepthWriteEnabled(true);
    mtl_state.depth_states[1] = mtl_state.device->newDepthStencilState(d);
    d->release();


    for (int linear_filter = 0; linear_filter < 2; linear_filter++) {
        for (int cms = 0; cms < 3; cms++) {
            for (int cmt = 0; cmt < 3; cmt++) {
                MTL::SamplerDescriptor *desc = MTL::SamplerDescriptor::alloc()->init();

                // Filter
                auto filter = linear_filter ? MTL::SamplerMinMagFilterLinear : MTL::SamplerMinMagFilterNearest;
                desc->setMinFilter(filter);
                desc->setMagFilter(filter);
                desc->setMipFilter(linear_filter ? MTL::SamplerMipFilterLinear : MTL::SamplerMipFilterNearest);

                // Address modes
                // Replace address_modes[cms/cmt] with your mapping: D3D12_WRAP -> MTL::SamplerAddressModeRepeat, etc.
                desc->setSAddressMode(cms == 0 ? MTL::SamplerAddressModeRepeat : (cms == 1 ? MTL::SamplerAddressModeMirrorRepeat : MTL::SamplerAddressModeClampToEdge));
                desc->setTAddressMode(cmt == 0 ? MTL::SamplerAddressModeRepeat : (cmt == 1 ? MTL::SamplerAddressModeMirrorRepeat : MTL::SamplerAddressModeClampToEdge));
                desc->setRAddressMode(MTL::SamplerAddressModeRepeat); // W always wrap

                // LOD / Anisotropy
                desc->setLodMinClamp(0.0f);
                desc->setLodMaxClamp(FLT_MAX);
                //desc->setLodBias(0.0f); 26 only i guess
                desc->setMaxAnisotropy(1);

                MTL::SamplerState *sampler = mtl_state.device->newSamplerState(desc);
                if (!sampler) {
                    std::cout << "failed to alloc sampler\n";
                    exit(-1);
                }
                desc->release();

                mtl_state.samplers.push_back(sampler); // store for later binding
            }
        }
    }
}

static void gfx_metal_on_resize(void) {
}

static void gfx_metal_start_frame(void) {
    mtl_state.autorelease_pool = NS::AutoreleasePool::alloc()->init();
    
    mtl_state.current_drawable = mtl_state.layer->nextDrawable();
    // do nothing for this frame if we can't get a drawable
    if (!mtl_state.current_drawable) {
        return;
    }

    if (mtl_state.needs_resize_depth) {
        create_depth_texture(mtl_state.viewport.width, mtl_state.viewport.height);
        mtl_state.needs_resize_depth = false;
    }

    mtl_state.current_pass_desc = MTL::RenderPassDescriptor::renderPassDescriptor();

    MetalUniforms *u = (MetalUniforms *)mtl_state.uniforms_buffer->contents();
    u->noise_frame++;
    u->noise_scale_x = mtl_state.viewport.width / 320.0f;
    u->noise_scale_y = mtl_state.viewport.height / 240.0f;
    if (u->noise_frame > 150) {
        u->noise_frame = 0;
    }

    auto color = mtl_state.current_pass_desc->colorAttachments()->object(0);
    color->setTexture(mtl_state.current_drawable->texture());
    color->setLoadAction(MTL::LoadActionClear);
    color->setStoreAction(MTL::StoreActionStore);
    color->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 1.0));

    auto depthAttachment = mtl_state.current_pass_desc->depthAttachment();
    depthAttachment->setTexture(mtl_state.depth_texture);
    depthAttachment->setLoadAction(MTL::LoadActionClear);
    depthAttachment->setStoreAction(MTL::StoreActionDontCare);
    depthAttachment->setClearDepth(1.0f);

    mtl_state.current_cmd_buffer = mtl_state.queue->commandBuffer();

    mtl_state.current_encoder = mtl_state.current_cmd_buffer->renderCommandEncoder(mtl_state.current_pass_desc);

    // make sure new encoder is configured correctly
    mtl_state.new_encoder = true;

    mtl_state.current_encoder->setVertexBuffer(
        mtl_state.dynamic_vertex_buffer,
        mtl_state.dynamic_offset,
        0
    );
}

static void gfx_metal_finish_render(void) {
}

static void gfx_metal_end_frame(void) {
    // like start_frame and draw triangles, skip this frame
    if (!mtl_state.current_drawable) {
        goto done;
    }

    mtl_state.current_encoder->endEncoding();

    mtl_state.current_cmd_buffer->presentDrawable(
        mtl_state.current_drawable);

    mtl_state.current_cmd_buffer->commit();
    // lazy way but it works
    mtl_state.current_cmd_buffer->waitUntilCompleted();
    mtl_state.dynamic_offset = 0;

    mtl_state.current_encoder = nullptr;
    mtl_state.current_drawable = nullptr;
    mtl_state.current_cmd_buffer = nullptr;
    mtl_state.current_pass_desc = nullptr;
done:
    mtl_state.autorelease_pool->drain();
    mtl_state.autorelease_pool = nullptr;
}

static void gfx_metal_shutdown(void) {
}

GfxRenderingAPI gfx_metal_api = {
    gfx_metal_z_is_from_0_to_1,
    gfx_metal_unload_shader,
    gfx_metal_load_shader,
    gfx_metal_create_and_load_new_shader,
    gfx_metal_lookup_shader,
    gfx_metal_shader_get_info,
    gfx_metal_new_texture,
    gfx_metal_select_texture,
    gfx_metal_upload_texture,
    gfx_metal_set_sampler_parameters,
    gfx_metal_set_depth_test,
    gfx_metal_set_depth_mask,
    gfx_metal_set_zmode_decal,
    gfx_metal_set_viewport,
    gfx_metal_set_scissor,
    gfx_metal_set_use_alpha,
    gfx_metal_draw_triangles,
    gfx_metal_init,
    gfx_metal_on_resize,
    gfx_metal_start_frame,
    gfx_metal_end_frame,
    gfx_metal_finish_render,
    gfx_metal_shutdown
};
#endif // RAPI_METAL
