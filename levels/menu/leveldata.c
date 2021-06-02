#include <ultra64.h>
#include "sm64.h"
#include "surface_terrains.h"
#include "moving_texture_macros.h"
#include "level_misc_macros.h"
#include "macro_preset_names.h"
#include "special_preset_names.h"
#include "textures.h"
#include "make_const_nonconst.h"

// 0x07000000 - 0x07000018
static const Lights1 lights_menu_save_button = gdSPDefLights1(
    0x3f, 0x3f, 0x3f,
    0xff, 0xff, 0xff, 0x28, 0x28, 0x28
);

// 0x07000018 - 0x07000818
ALIGNED8 static const u8 texture_menu_stone[] = "levels/menu/main_menu_seg7.00018.rgba16";

// 0x07000818 - 0x07001018
ALIGNED8 static const u8 texture_menu_dark_stone[] = "levels/menu/main_menu_seg7.00818.rgba16";

// 0x07001018 - 0x07002018
ALIGNED8 static const u8 texture_menu_mario_save[] = "levels/menu/main_menu_seg7.01018.rgba16";

// 0x07002018 - 0x07003018
ALIGNED8 static const u8 texture_menu_mario_new[] = "levels/menu/main_menu_seg7.02018.rgba16";

// 0x07003018 - 0x07003118
static const Vtx vertex_menu_save_button_borders[] = {
    {{{  -163,   -122,      0}, 0, {     0,    990}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{   163,   -122,      0}, 0, {   990,    990}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{  -122,    -81,     30}, 0, {    96,    820}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{   122,    -81,     30}, 0, {   862,    820}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{  -163,   -122,      0}, 0, {     0,    990}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -122,    -81,     30}, 0, {    96,    820}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -163,    122,      0}, 0, {     0,      0}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -122,     81,     30}, 0, {    96,    138}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -122,     81,     30}, 0, {    96,    138}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{   122,     81,     30}, 0, {   862,    138}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{   163,    122,      0}, 0, {   990,      0}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{  -163,    122,      0}, 0, {     0,      0}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{   122,     81,     30}, 0, {   862,    138}, {0x4a, 0x00, 0x66, 0xff}}},
    {{{   122,    -81,     30}, 0, {   862,    820}, {0x4a, 0x00, 0x66, 0xff}}},
    {{{   163,   -122,      0}, 0, {   990,    990}, {0x4a, 0x00, 0x66, 0xff}}},
    {{{   163,    122,      0}, 0, {   990,      0}, {0x4a, 0x00, 0x66, 0xff}}},
};

// 0x07003118 - 0x07003158
static const Vtx vertex_menu_save_button_front[] = {
    {{{   122,     81,     30}, 0, {  2012,      0}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{  -122,     81,     30}, 0, {     0,      0}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{   122,    -81,     30}, 0, {  2012,    990}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{  -122,    -81,     30}, 0, {     0,    990}, {0x00, 0x00, 0x7f, 0xff}}},
};

// 0x07003158 - 0x070031A0
static const Gfx dl_tex_block_menu_save_button_base[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
    gsSPClearGeometryMode(G_SHADING_SMOOTH),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x070031A0 - 0x07003218
static const Gfx dl_vertex_menu_save_button_borders[] = {
    gsSPLight(&lights_menu_save_button.l, 1),
    gsSPLight(&lights_menu_save_button.a, 2),
    gsSPVertex(vertex_menu_save_button_borders, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  1,  3,  2, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  5,  7,  6, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0, 11,  8, 10, 0x0),
    gsSP2Triangles(12, 13, 14, 0x0, 15, 12, 14, 0x0),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 6, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x07003218 - 0x07003258
static const Gfx dl_vertex_menu_save_button_front[] = {
    gsSPVertex(vertex_menu_save_button_front, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  1,  3,  2, 0x0),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPSetGeometryMode(G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// 0x07003258 - 0x07003298
static const Vtx vertex_menu_save_button_back[] = {
    {{{   163,   -122,      0}, 0, {     0,    990}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -163,   -122,      0}, 0, {   990,    990}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   163,    122,      0}, 0, {     0,      0}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -163,    122,      0}, 0, {   990,      0}, {0x00, 0x00, 0x81, 0xff}}},
};

// 0x07003298 - 0x070032E0
static const Gfx dl_tex_block_menu_save_button_back[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
    gsSPClearGeometryMode(G_SHADING_SMOOTH),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x070032E0 - 0x07003330
static const Gfx dl_vertex_menu_save_button_back[] = {
    gsSPLight(&lights_menu_save_button.l, 1),
    gsSPLight(&lights_menu_save_button.a, 2),
    gsSPVertex(vertex_menu_save_button_back, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  1,  3,  2, 0x0),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPSetGeometryMode(G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// 0x07003330 - 0x07003380
const Gfx dl_menu_mario_save_button_base[] = {
    gsSPDisplayList(dl_tex_block_menu_save_button_base),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_stone),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_vertex_menu_save_button_borders),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_mario_save),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 64 * 32 - 1, CALC_DXT(64, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_vertex_menu_save_button_front),
    gsSPEndDisplayList(),
};

// 0x07003380 - 0x070033D0
const Gfx dl_menu_mario_new_button_base[] = {
    gsSPDisplayList(dl_tex_block_menu_save_button_base),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_stone),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_vertex_menu_save_button_borders),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_mario_new),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 64 * 32 - 1, CALC_DXT(64, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_vertex_menu_save_button_front),
    gsSPEndDisplayList(),
};

// 0x070033D0 - 0x07003400
const Gfx dl_menu_save_button_back[] = {
    gsSPDisplayList(dl_tex_block_menu_save_button_back),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_dark_stone),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_vertex_menu_save_button_back),
    gsSPEndDisplayList(),
};

// 0x07003400 - 0x07003450
const Gfx dl_menu_save_button_fade_back[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_SHADING_SMOOTH),
    gsSPLight(&lights_menu_save_button.l, 1),
    gsSPLight(&lights_menu_save_button.a, 2),
    gsSPVertex(vertex_menu_save_button_back, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  1,  3,  2, 0x0),
    gsDPPipeSync(),
    gsSPSetGeometryMode(G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// 0x07003450 - 0x07003468
static const Lights1 lights_menu_main_button = gdSPDefLights1(
    0x3f, 0x3f, 0x3f,
    0xff, 0xff, 0xff, 0x28, 0x28, 0x28
);

// 0x07003468 - 0x07003468
ALIGNED8 static const u8 texture_menu_erase[] = "levels/menu/main_menu_seg7.03468.rgba16";

// 0x07003C68 - 0x07003C68
ALIGNED8 static const u8 texture_menu_copy[] = "levels/menu/main_menu_seg7.03C68.rgba16";

// 0x07004468 - 0x07004468
ALIGNED8 static const u8 texture_menu_file[] = "levels/menu/main_menu_seg7.04468.rgba16";

// 0x07004C68 - 0x07004C68
ALIGNED8 static const u8 texture_menu_score[] = "levels/menu/main_menu_seg7.04C68.rgba16";

// 0x07005468 - 0x07005468
ALIGNED8 static const u8 texture_menu_sound[] = "levels/menu/main_menu_seg7.05468.rgba16";

// 0x07005C68 - 0x07005D68
static const Vtx vertex_menu_main_button_group1[] = {
    {{{  -163,   -122,      0}, 0, {   990,      0}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -122,    -81,     30}, 0, {   862,    138}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -163,    122,      0}, 0, {   990,    990}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -143,    102,      0}, 0, {   926,    904}, {0x59, 0x00, 0xa7, 0xff}}},
    {{{  -133,     92,     10}, 0, {   894,    862}, {0x59, 0x00, 0xa7, 0xff}}},
    {{{  -133,    -92,     10}, 0, {   894,     96}, {0x59, 0x00, 0xa7, 0xff}}},
    {{{  -133,     92,     10}, 0, {   894,    862}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   133,    -92,     10}, 0, {    64,     96}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -133,    -92,     10}, 0, {   894,     96}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   133,     92,     10}, 0, {    64,    862}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   133,     92,     10}, 0, {    64,    862}, {0x00, 0xa7, 0xa7, 0xff}}},
    {{{  -133,     92,     10}, 0, {   894,    862}, {0x00, 0xa7, 0xa7, 0xff}}},
    {{{  -143,    102,      0}, 0, {   926,    904}, {0x00, 0xa7, 0xa7, 0xff}}},
    {{{   143,   -102,      0}, 0, {    32,     54}, {0xa7, 0x00, 0xa7, 0xff}}},
    {{{   133,     92,     10}, 0, {    64,    862}, {0xa7, 0x00, 0xa7, 0xff}}},
    {{{   143,    102,      0}, 0, {    32,    904}, {0xa7, 0x00, 0xa7, 0xff}}},
};

// 0x07005D68 - 0x07005E68
static const Vtx vertex_menu_main_button_group2[] = {
    {{{   143,   -102,      0}, 0, {    32,     54}, {0xa7, 0x00, 0xa7, 0xff}}},
    {{{   133,    -92,     10}, 0, {    64,     96}, {0xa7, 0x00, 0xa7, 0xff}}},
    {{{   133,     92,     10}, 0, {    64,    862}, {0xa7, 0x00, 0xa7, 0xff}}},
    {{{   133,     92,     10}, 0, {    64,    862}, {0x00, 0xa7, 0xa7, 0xff}}},
    {{{  -143,    102,      0}, 0, {   926,    904}, {0x00, 0xa7, 0xa7, 0xff}}},
    {{{   143,    102,      0}, 0, {    32,    904}, {0x00, 0xa7, 0xa7, 0xff}}},
    {{{  -143,   -102,      0}, 0, {   926,     54}, {0x00, 0x59, 0xa7, 0xff}}},
    {{{   133,    -92,     10}, 0, {    64,     96}, {0x00, 0x59, 0xa7, 0xff}}},
    {{{   143,   -102,      0}, 0, {    32,     54}, {0x00, 0x59, 0xa7, 0xff}}},
    {{{  -133,    -92,     10}, 0, {   894,     96}, {0x00, 0x59, 0xa7, 0xff}}},
    {{{  -143,    102,      0}, 0, {   926,    904}, {0x59, 0x00, 0xa7, 0xff}}},
    {{{  -133,    -92,     10}, 0, {   894,     96}, {0x59, 0x00, 0xa7, 0xff}}},
    {{{  -143,   -102,      0}, 0, {   926,     54}, {0x59, 0x00, 0xa7, 0xff}}},
    {{{   163,    122,      0}, 0, {     0,    990}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -143,    102,      0}, 0, {   926,    904}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -163,    122,      0}, 0, {   990,    990}, {0x00, 0x00, 0x81, 0xff}}},
};

// 0x07005E68 - 0x07005F48
static const Vtx vertex_menu_main_button_group3[] = {
    {{{   163,    122,      0}, 0, {     0,    990}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   143,    102,      0}, 0, {    32,    904}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -143,    102,      0}, 0, {   926,    904}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   143,   -102,      0}, 0, {    32,     54}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   163,   -122,      0}, 0, {     0,      0}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -163,    122,      0}, 0, {   990,    990}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -143,   -102,      0}, 0, {   926,     54}, {0x00, 0x00, 0x81, 0xff}}},
    {{{  -163,   -122,      0}, 0, {   990,      0}, {0x00, 0x00, 0x81, 0xff}}},
    {{{   163,   -122,      0}, 0, {     0,      0}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{   122,    -81,     30}, 0, {    96,    138}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{  -122,    -81,     30}, 0, {   862,    138}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{  -122,    -81,     30}, 0, {   862,    138}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -122,     81,     30}, 0, {   862,    820}, {0xb6, 0x00, 0x66, 0xff}}},
    {{{  -163,    122,      0}, 0, {   990,    990}, {0xb6, 0x00, 0x66, 0xff}}},
};

// 0x07005F48 - 0x07006038
static const Vtx vertex_menu_main_button_group4[] = {
    {{{  -122,     81,     30}, 0, {   862,    820}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{  -122,    -81,     30}, 0, {   862,    138}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{   122,    -81,     30}, 0, {    96,    138}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{  -163,   -122,      0}, 0, {   990,      0}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{   163,   -122,      0}, 0, {     0,      0}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{  -122,    -81,     30}, 0, {   862,    138}, {0x00, 0xb6, 0x66, 0xff}}},
    {{{  -122,     81,     30}, 0, {   862,    820}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{   122,     81,     30}, 0, {    96,    820}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{   163,    122,      0}, 0, {     0,    990}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{  -163,    122,      0}, 0, {   990,    990}, {0x00, 0x4a, 0x66, 0xff}}},
    {{{   122,     81,     30}, 0, {    96,    820}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{   163,    122,      0}, 0, {     0,    990}, {0x4a, 0x00, 0x66, 0xff}}},
    {{{   122,     81,     30}, 0, {    96,    820}, {0x4a, 0x00, 0x66, 0xff}}},
    {{{   163,   -122,      0}, 0, {     0,      0}, {0x4a, 0x00, 0x66, 0xff}}},
    {{{   122,    -81,     30}, 0, {    96,    138}, {0x4a, 0x00, 0x66, 0xff}}},
};

// 0x07006038 - 0x07006150
static const Gfx dl_vertex_menu_main_button[] = {
    gsSPLight(&lights_menu_main_button.l, 1),
    gsSPLight(&lights_menu_main_button.a, 2),
    gsSPVertex(vertex_menu_main_button_group1, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  9,  7, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(vertex_menu_main_button_group2, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  9,  7, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(vertex_menu_main_button_group3, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 0,  4,  3, 0x0,  5,  2,  6, 0x0),
    gsSP2Triangles( 5,  6,  7, 0x0,  6,  3,  4, 0x0),
    gsSP2Triangles( 6,  4,  7, 0x0,  8,  9, 10, 0x0),
    gsSP1Triangle(11, 12, 13, 0x0),
    gsSPVertex(vertex_menu_main_button_group4, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9,  6,  8, 0x0),
    gsSP2Triangles(10,  0,  2, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(12, 14, 13, 0x0),
    gsSPEndDisplayList(),
};

// 0x07006150 - 0x07006198
static const Gfx dl_tex_block_menu_main_button[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
    gsSPClearGeometryMode(G_SHADING_SMOOTH),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x07006198 - 0x070061C8
static const Gfx dl_menu_main_button[] = {
    gsSPDisplayList(dl_vertex_menu_main_button),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPSetGeometryMode(G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// 0x070061C8 - 0x070061F8
const Gfx dl_menu_erase_button[] = {
    gsSPDisplayList(dl_tex_block_menu_main_button),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_erase),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_menu_main_button),
    gsSPEndDisplayList(),
};

// 0x070061F8 - 0x07006228
const Gfx dl_menu_copy_button[] = {
    gsSPDisplayList(dl_tex_block_menu_main_button),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_copy),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_menu_main_button),
    gsSPEndDisplayList(),
};

// 0x07006228 - 0x07006258
const Gfx dl_menu_file_button[] = {
    gsSPDisplayList(dl_tex_block_menu_main_button),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_file),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_menu_main_button),
    gsSPEndDisplayList(),
};

// 0x07006258 - 0x07006288
const Gfx dl_menu_score_button[] = {
    gsSPDisplayList(dl_tex_block_menu_main_button),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_score),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_menu_main_button),
    gsSPEndDisplayList(),
};

// 0x07006288 - 0x070062B8
const Gfx dl_menu_sound_button[] = {
    gsSPDisplayList(dl_tex_block_menu_main_button),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_sound),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_menu_main_button),
    gsSPEndDisplayList(),
};

// 0x070062B8 - 0x070062E8
const Gfx dl_menu_generic_button[] = {
    gsSPDisplayList(dl_tex_block_menu_main_button),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_stone),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_menu_main_button),
    gsSPEndDisplayList(),
};

// 0x070062E8 - 0x07006328
static const Vtx vertex_menu_hand[] = {
    {{{     0,      0,      0}, 0, {     0,   1984}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{    32,      0,      0}, 0, {  1984,   1984}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{    32,     32,      0}, 0, {  1984,      0}, {0x00, 0x00, 0x7f, 0xff}}},
    {{{     0,     32,      0}, 0, {     0,      0}, {0x00, 0x00, 0x7f, 0xff}}},
};

// 0x07006328 - 0x07006B28
ALIGNED8 static const u8 texture_menu_idle_hand[] = "levels/menu/main_menu_seg7.06328.rgba16";

// 0x07006B28 - 0x07007328
ALIGNED8 static const u8 texture_menu_grabbing_hand[] = "levels/menu/main_menu_seg7.06B28.rgba16";

// 0x07007328 - 0x070073A0
static const Gfx dl_menu_hand[] = {
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetRenderMode(G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPVertex(vertex_menu_hand, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSPTexture(0x0001, 0x0001, 0, G_TX_RENDERTILE, G_OFF),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x070073A0 - 0x070073B8
const Gfx dl_menu_idle_hand[] = {
    gsDPPipeSync(),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_idle_hand),
    gsSPBranchList(dl_menu_hand),
};

// 0x070073B8 - 0x070073D0
const Gfx dl_menu_grabbing_hand[] = {
    gsDPPipeSync(),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_grabbing_hand),
    gsSPBranchList(dl_menu_hand),
};

// 0x0700D108 - 0x0700D160
const Gfx dl_menu_ia8_text_begin[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetCombineMode(G_CC_FADEA, G_CC_FADEA),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetRenderMode(G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 1, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 3, G_TX_NOLOD, G_TX_CLAMP, 3, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (8 - 1) << G_TEXTURE_IMAGE_FRAC, (8 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x0700D160 - 0x0700D1A0
const Gfx dl_menu_ia8_text_end[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPEndDisplayList(),
};

UNUSED static const u64 menu_unused_1 = 0;

// 0x0700D1A8 - 0x0700E1A8
ALIGNED8 static const u8 texture_menu_course_upper[] = "levels/menu/main_menu_seg7.0D1A8.rgba16";

// 0x0700E1A8 - 0x0700F1A8
ALIGNED8 static const u8 texture_menu_course_lower[] = "levels/menu/main_menu_seg7.0E1A8.rgba16";

// 0x0700F1A8 - 0x0700F1E8
static const Vtx vertex_menu_course_upper[] = {
    {{{   -32,      0,      0}, 0, {     0,   1984}, {0x00, 0x00, 0x7f, 0x00}}},
    {{{    32,      0,      0}, 0, {  4032,   1984}, {0x00, 0x00, 0x7f, 0x00}}},
    {{{    32,     32,      0}, 0, {  4032,      0}, {0x00, 0x00, 0x7f, 0x00}}},
    {{{   -32,     32,      0}, 0, {     0,      0}, {0x00, 0x00, 0x7f, 0x00}}},
};

// 0x0700F1E8 - 0x0700F228
static const Vtx vertex_menu_course_lower[] = {
    {{{   -32,    -32,      0}, 0, {     0,   1984}, {0x00, 0x00, 0x7f, 0x00}}},
    {{{    32,    -32,      0}, 0, {  4032,   1984}, {0x00, 0x00, 0x7f, 0x00}}},
    {{{    32,      0,      0}, 0, {  4032,      0}, {0x00, 0x00, 0x7f, 0x00}}},
    {{{   -32,      0,      0}, 0, {     0,      0}, {0x00, 0x00, 0x7f, 0x00}}},
};

// 0x0700F228 - 0x0700F2F8
const Gfx dl_menu_rgba16_wood_course[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetRenderMode(G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_course_upper),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 64 * 32 - 1, CALC_DXT(64, G_IM_SIZ_16b_BYTES)),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPVertex(vertex_menu_course_upper, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_menu_course_lower),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 64 * 32 - 1, CALC_DXT(64, G_IM_SIZ_16b_BYTES)),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPVertex(vertex_menu_course_lower, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsSPTexture(0x0001, 0x0001, 0, G_TX_RENDERTILE, G_OFF),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x0700F2F8 - 0x0700F328
const Collision main_menu_seg7_collision[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0x4),
    COL_VERTEX( 8192, -1000, -8192),
    COL_VERTEX(-8192, -1000, -8192),
    COL_VERTEX(-8192, -1000,  8192),
    COL_VERTEX( 8192, -1000,  8192),
    COL_TRI_INIT(SURFACE_DEFAULT, 2),
    COL_TRI(0, 1, 2),
    COL_TRI(0, 2, 3),
    COL_TRI_STOP(),
    COL_END(),
};
