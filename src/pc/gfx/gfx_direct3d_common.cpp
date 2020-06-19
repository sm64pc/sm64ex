#if (defined(RAPI_D3D11) || defined(RAPI_D3D12)) && (defined(_WIN32) || defined(_WIN64))

#include <cstdio>

extern "C" {
#include "../platform.h"
}

#include "gfx_direct3d_common.h"
#include "gfx_cc.h"

void ThrowIfFailed(HRESULT res) {
    if (FAILED(res))
        sys_fatal("error while initializing D3D:\nerror code 0x%08X", res);
}

void ThrowIfFailed(HRESULT res, HWND h_wnd, const char *message) {
    if (FAILED(res))
        sys_fatal("%s\nerror code 0x%08X", message, res);
}

void get_cc_features(uint32_t shader_id, CCFeatures *cc_features) {
    for (int i = 0; i < 4; i++) {
        cc_features->c[0][i] = (shader_id >> (i * 3)) & 7;
        cc_features->c[1][i] = (shader_id >> (12 + i * 3)) & 7;
    }

    cc_features->opt_alpha = (shader_id & SHADER_OPT_ALPHA) != 0;
    cc_features->opt_fog = (shader_id & SHADER_OPT_FOG) != 0;
    cc_features->opt_texture_edge = (shader_id & SHADER_OPT_TEXTURE_EDGE) != 0;
    cc_features->opt_noise = (shader_id & SHADER_OPT_NOISE) != 0;

    cc_features->used_textures[0] = false;
    cc_features->used_textures[1] = false;
    cc_features->num_inputs = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (cc_features->c[i][j] >= SHADER_INPUT_1 && cc_features->c[i][j] <= SHADER_INPUT_4) {
                if (cc_features->c[i][j] > cc_features->num_inputs) {
                    cc_features->num_inputs = cc_features->c[i][j];
                }
            }
            if (cc_features->c[i][j] == SHADER_TEXEL0 || cc_features->c[i][j] == SHADER_TEXEL0A) {
                cc_features->used_textures[0] = true;
            }
            if (cc_features->c[i][j] == SHADER_TEXEL1) {
                cc_features->used_textures[1] = true;
            }
        }
    }

    cc_features->do_single[0] = cc_features->c[0][2] == 0;
    cc_features->do_single[1] = cc_features->c[1][2] == 0;
    cc_features->do_multiply[0] = cc_features->c[0][1] == 0 && cc_features->c[0][3] == 0;
    cc_features->do_multiply[1] = cc_features->c[1][1] == 0 && cc_features->c[1][3] == 0;
    cc_features->do_mix[0] = cc_features->c[0][1] == cc_features->c[0][3];
    cc_features->do_mix[1] = cc_features->c[1][1] == cc_features->c[1][3];
    cc_features->color_alpha_same = (shader_id & 0xfff) == ((shader_id >> 12) & 0xfff);
}

void append_str(char *buf, size_t *len, const char *str) {
    while (*str != '\0') buf[(*len)++] = *str++;
}

void append_line(char *buf, size_t *len, const char *str) {
    while (*str != '\0') buf[(*len)++] = *str++;
    buf[(*len)++] = '\r';
    buf[(*len)++] = '\n';
}

const char *shader_item_to_str(uint32_t item, bool with_alpha, bool only_alpha, bool inputs_have_alpha, bool hint_single_element) {
    if (!only_alpha) {
        switch (item) {
            default:
            case SHADER_0:
                return with_alpha ? "float4(0.0, 0.0, 0.0, 0.0)" : "float3(0.0, 0.0, 0.0)";
            case SHADER_INPUT_1:
                return with_alpha || !inputs_have_alpha ? "input.input1" : "input.input1.rgb";
            case SHADER_INPUT_2:
                return with_alpha || !inputs_have_alpha ? "input.input2" : "input.input2.rgb";
            case SHADER_INPUT_3:
                return with_alpha || !inputs_have_alpha ? "input.input3" : "input.input3.rgb";
            case SHADER_INPUT_4:
                return with_alpha || !inputs_have_alpha ? "input.input4" : "input.input4.rgb";
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
                return "input.input1.a";
            case SHADER_INPUT_2:
                return "input.input2.a";
            case SHADER_INPUT_3:
                return "input.input3.a";
            case SHADER_INPUT_4:
                return "input.input4.a";
            case SHADER_TEXEL0:
                return "texVal0.a";
            case SHADER_TEXEL0A:
                return "texVal0.a";
            case SHADER_TEXEL1:
                return "texVal1.a";
        }
    }
}

void append_formula(char *buf, size_t *len, uint8_t c[2][4], bool do_single, bool do_multiply, bool do_mix, bool with_alpha, bool only_alpha, bool opt_alpha) {
    if (do_single) {
        append_str(buf, len, shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    } else if (do_multiply) {
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " * ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
    } else if (do_mix) {
        append_str(buf, len, "lerp(");
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

#endif
