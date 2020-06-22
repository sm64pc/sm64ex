#if defined(RAPI_D3D11) || defined(RAPI_D3D12)

#ifndef GFX_DIRECT3D_COMMON_H
#define GFX_DIRECT3D_COMMON_H

#include <stdint.h>
#include <windows.h>

struct CCFeatures {
    uint8_t c[2][4];
    bool opt_alpha;
    bool opt_fog;
    bool opt_texture_edge;
    bool opt_noise;
    bool used_textures[2];
    uint32_t num_inputs;
    bool do_single[2];
    bool do_multiply[2];
    bool do_mix[2];
    bool color_alpha_same;
};

void ThrowIfFailed(HRESULT res);
void ThrowIfFailed(HRESULT res, HWND h_wnd, const char *message);
void get_cc_features(uint32_t shader_id, CCFeatures *shader_features);
void append_str(char *buf, size_t *len, const char *str);
void append_line(char *buf, size_t *len, const char *str);
const char *shader_item_to_str(uint32_t item, bool with_alpha, bool only_alpha, bool inputs_have_alpha, bool hint_single_element);
void append_formula(char *buf, size_t *len, uint8_t c[2][4], bool do_single, bool do_multiply, bool do_mix, bool with_alpha, bool only_alpha, bool opt_alpha);

#endif

#endif
