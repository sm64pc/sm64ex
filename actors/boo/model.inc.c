#include "actors/group9.h"

// Boo

// 0x05009B28
static const Lights1 boo_seg5_lights_05009B28 = gdSPDefLights1(
    0x97, 0x9a, 0xff,
    0xff, 0xff, 0xff, 0x28, 0x28, 0x28
);

// 0x05009B40
ALIGNED8 static const u8 boo_seg5_texture_05009B40[] = {
#include "actors/boo/boo_eyes.rgba16.inc.c"
};

// 0x0500AB40
ALIGNED8 static const u8 boo_seg5_texture_0500AB40[] = {
#include "actors/boo/boo_mouth.rgba16.inc.c"
};

// 0x0500B340
static const Vtx boo_seg5_vertex_0500B340[] = {
    {{{     0,   -117,    131}, 0, {   458,    990}, {0x00, 0x9d, 0x4e, 0x9e}}},
    {{{    87,    -78,    123}, 0, {  1096,    684}, {0x53, 0xc9, 0x4e, 0x9e}}},
    {{{     0,    -47,    171}, 0, {   458,    168}, {0x00, 0xe9, 0x7c, 0x9e}}},
    {{{     0,   -117,    131}, 0, {   458,    990}, {0x00, 0x9d, 0x4e, 0x9e}}},
    {{{     0,    -47,    171}, 0, {   458,    168}, {0x00, 0xe9, 0x7c, 0x9e}}},
    {{{   -86,    -78,    123}, 0, {  -176,    684}, {0xad, 0xc9, 0x4e, 0x9e}}},
    {{{    87,    -78,    123}, 0, {  1096,    684}, {0x53, 0xc9, 0x4e, 0x9e}}},
    {{{   108,     20,    118}, 0, {  1248,   -172}, {0x57, 0x14, 0x5a, 0x9e}}},
    {{{     0,    -47,    171}, 0, {   458,    168}, {0x00, 0xe9, 0x7c, 0x9e}}},
    {{{   -86,    -78,    123}, 0, {  -176,    684}, {0xad, 0xc9, 0x4e, 0x9e}}},
    {{{     0,    -47,    171}, 0, {   458,    168}, {0x00, 0xe9, 0x7c, 0x9e}}},
    {{{  -107,     20,    118}, 0, {  -328,   -172}, {0xa9, 0x14, 0x59, 0x9e}}},
};

// 0x0500B400
static const Vtx boo_seg5_vertex_0500B400[] = {
    {{{  -107,     20,    118}, 0, {  -656,    538}, {0xa9, 0x14, 0x59, 0x9e}}},
    {{{     0,     60,    166}, 0, {   988,    148}, {0x00, 0x26, 0x79, 0x9e}}},
    {{{   -57,    128,    108}, 0, {    76,   -690}, {0xd3, 0x60, 0x45, 0x9e}}},
    {{{  -107,     20,    118}, 0, {  -656,    538}, {0xa9, 0x14, 0x59, 0x9e}}},
    {{{     0,    -47,    171}, 0, {  1024,   1364}, {0x00, 0xe9, 0x7c, 0x9e}}},
    {{{     0,     60,    166}, 0, {   988,    148}, {0x00, 0x26, 0x79, 0x9e}}},
    {{{     0,     60,    166}, 0, {   988,    148}, {0x00, 0x26, 0x79, 0x9e}}},
    {{{     0,    -47,    171}, 0, {  1024,   1364}, {0x00, 0xe9, 0x7c, 0x9e}}},
    {{{   108,     20,    118}, 0, {  2660,    540}, {0x57, 0x14, 0x5a, 0x9e}}},
    {{{   108,     20,    118}, 0, {  2660,    540}, {0x57, 0x14, 0x5a, 0x9e}}},
    {{{    58,    128,    108}, 0, {  1852,   -688}, {0x35, 0x58, 0x49, 0x9e}}},
    {{{     0,     60,    166}, 0, {   988,    148}, {0x00, 0x26, 0x79, 0x9e}}},
};

// 0x0500B4C0
static const Vtx boo_seg5_vertex_0500B4C0[] = {
    {{{  -135,    -70,     23}, 0, {     0,      0}, {0xb2, 0xaa, 0x33, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{    73,   -138,     30}, 0, {     0,      0}, {0x39, 0x90, 0x0e, 0x9e}}},
    {{{     0,   -117,    131}, 0, {     0,      0}, {0x00, 0x9d, 0x4e, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{   -86,    -78,    123}, 0, {     0,      0}, {0xad, 0xc9, 0x4e, 0x9e}}},
    {{{  -135,    -70,     23}, 0, {     0,      0}, {0xb2, 0xaa, 0x33, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{   -59,   -126,    -86}, 0, {     0,      0}, {0xe1, 0x90, 0xd0, 0x9e}}},
    {{{    60,   -126,    -86}, 0, {     0,      0}, {0x20, 0x8b, 0xdb, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{    60,   -126,    -86}, 0, {     0,      0}, {0x20, 0x8b, 0xdb, 0x9e}}},
    {{{    73,   -138,     30}, 0, {     0,      0}, {0x39, 0x90, 0x0e, 0x9e}}},
};

// 0x0500B5B0
static const Vtx boo_seg5_vertex_0500B5B0[] = {
    {{{   -86,    -78,    123}, 0, {     0,      0}, {0xad, 0xc9, 0x4e, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{     0,   -117,    131}, 0, {     0,      0}, {0x00, 0x9d, 0x4e, 0x9e}}},
    {{{   -59,   -126,    -86}, 0, {     0,      0}, {0xe1, 0x90, 0xd0, 0x9e}}},
    {{{   -72,   -138,     30}, 0, {     0,      0}, {0xcf, 0x8c, 0x0a, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
    {{{    60,   -126,    -86}, 0, {     0,      0}, {0x20, 0x8b, 0xdb, 0x9e}}},
    {{{   -59,   -126,    -86}, 0, {     0,      0}, {0xe1, 0x90, 0xd0, 0x9e}}},
    {{{    46,    -82,   -160}, 0, {     0,      0}, {0x2e, 0xba, 0xa2, 0x9e}}},
    {{{    46,    -82,   -160}, 0, {     0,      0}, {0x2e, 0xba, 0xa2, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{    60,   -126,    -86}, 0, {     0,      0}, {0x20, 0x8b, 0xdb, 0x9e}}},
    {{{    60,   -126,    -86}, 0, {     0,      0}, {0x20, 0x8b, 0xdb, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{    73,   -138,     30}, 0, {     0,      0}, {0x39, 0x90, 0x0e, 0x9e}}},
};

// 0x0500B6A0
static const Vtx boo_seg5_vertex_0500B6A0[] = {
    {{{   136,    -70,     23}, 0, {     0,      0}, {0x4e, 0xaa, 0x33, 0x9e}}},
    {{{    73,   -138,     30}, 0, {     0,      0}, {0x39, 0x90, 0x0e, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{    87,    -78,    123}, 0, {     0,      0}, {0x53, 0xc9, 0x4e, 0x9e}}},
    {{{    73,   -138,     30}, 0, {     0,      0}, {0x39, 0x90, 0x0e, 0x9e}}},
    {{{   136,    -70,     23}, 0, {     0,      0}, {0x4e, 0xaa, 0x33, 0x9e}}},
    {{{    87,    -78,    123}, 0, {     0,      0}, {0x53, 0xc9, 0x4e, 0x9e}}},
    {{{     0,   -117,    131}, 0, {     0,      0}, {0x00, 0x9d, 0x4e, 0x9e}}},
    {{{    73,   -138,     30}, 0, {     0,      0}, {0x39, 0x90, 0x0e, 0x9e}}},
    {{{   -43,    162,     15}, 0, {     0,      0}, {0xdf, 0x7a, 0xf8, 0x9e}}},
    {{{   -57,    128,    108}, 0, {     0,      0}, {0xd3, 0x60, 0x45, 0x9e}}},
    {{{    44,    162,     15}, 0, {     0,      0}, {0x21, 0x7a, 0x09, 0x9e}}},
    {{{   -43,    162,     15}, 0, {     0,      0}, {0xdf, 0x7a, 0xf8, 0x9e}}},
    {{{    44,    162,     15}, 0, {     0,      0}, {0x21, 0x7a, 0x09, 0x9e}}},
    {{{    42,    130,    -88}, 0, {     0,      0}, {0x25, 0x6a, 0xc7, 0x9e}}},
};

// 0x0500B790
static const Vtx boo_seg5_vertex_0500B790[] = {
    {{{   125,     99,     15}, 0, {     0,      0}, {0x65, 0x4c, 0x08, 0x9e}}},
    {{{    44,    162,     15}, 0, {     0,      0}, {0x21, 0x7a, 0x09, 0x9e}}},
    {{{    58,    128,    108}, 0, {     0,      0}, {0x35, 0x58, 0x49, 0x9e}}},
    {{{    58,    128,    108}, 0, {     0,      0}, {0x35, 0x58, 0x49, 0x9e}}},
    {{{    44,    162,     15}, 0, {     0,      0}, {0x21, 0x7a, 0x09, 0x9e}}},
    {{{   -57,    128,    108}, 0, {     0,      0}, {0xd3, 0x60, 0x45, 0x9e}}},
    {{{    44,    162,     15}, 0, {     0,      0}, {0x21, 0x7a, 0x09, 0x9e}}},
    {{{   125,     99,     15}, 0, {     0,      0}, {0x65, 0x4c, 0x08, 0x9e}}},
    {{{    42,    130,    -88}, 0, {     0,      0}, {0x25, 0x6a, 0xc7, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
    {{{    42,    130,    -88}, 0, {     0,      0}, {0x25, 0x6a, 0xc7, 0x9e}}},
    {{{   125,     99,     15}, 0, {     0,      0}, {0x65, 0x4c, 0x08, 0x9e}}},
    {{{     0,     62,   -162}, 0, {     0,      0}, {0x00, 0x5a, 0xa8, 0x9e}}},
    {{{    42,    130,    -88}, 0, {     0,      0}, {0x25, 0x6a, 0xc7, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
};

// 0x0500B880
static const Vtx boo_seg5_vertex_0500B880[] = {
    {{{   -41,    130,    -88}, 0, {     0,      0}, {0xd4, 0x65, 0xc3, 0x9e}}},
    {{{   -43,    162,     15}, 0, {     0,      0}, {0xdf, 0x7a, 0xf8, 0x9e}}},
    {{{    42,    130,    -88}, 0, {     0,      0}, {0x25, 0x6a, 0xc7, 0x9e}}},
    {{{     0,     62,   -162}, 0, {     0,      0}, {0x00, 0x5a, 0xa8, 0x9e}}},
    {{{   -41,    130,    -88}, 0, {     0,      0}, {0xd4, 0x65, 0xc3, 0x9e}}},
    {{{    42,    130,    -88}, 0, {     0,      0}, {0x25, 0x6a, 0xc7, 0x9e}}},
    {{{  -135,    -70,     23}, 0, {     0,      0}, {0xb2, 0xaa, 0x33, 0x9e}}},
    {{{   -86,    -78,    123}, 0, {     0,      0}, {0xad, 0xc9, 0x4e, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{   -86,    -78,    123}, 0, {     0,      0}, {0xad, 0xc9, 0x4e, 0x9e}}},
    {{{  -107,     20,    118}, 0, {     0,      0}, {0xa9, 0x14, 0x59, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{  -199,    -60,     25}, 0, {     0,      0}, {0xa2, 0xbf, 0x36, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
    {{{  -135,    -70,     23}, 0, {     0,      0}, {0xb2, 0xaa, 0x33, 0x9e}}},
};

// 0x0500B970
static const Vtx boo_seg5_vertex_0500B970[] = {
    {{{  -107,     20,    118}, 0, {     0,      0}, {0xa9, 0x14, 0x59, 0x9e}}},
    {{{   -57,    128,    108}, 0, {     0,      0}, {0xd3, 0x60, 0x45, 0x9e}}},
    {{{  -124,     99,     15}, 0, {     0,      0}, {0x9b, 0x4c, 0x08, 0x9e}}},
    {{{  -124,     99,     15}, 0, {     0,      0}, {0x9b, 0x4c, 0x08, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{  -107,     20,    118}, 0, {     0,      0}, {0xa9, 0x14, 0x59, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{  -124,     99,     15}, 0, {     0,      0}, {0x9b, 0x4c, 0x08, 0x9e}}},
    {{{  -199,    -60,     25}, 0, {     0,      0}, {0xa2, 0xbf, 0x36, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
};

// 0x0500BA60
static const Vtx boo_seg5_vertex_0500BA60[] = {
    {{{  -199,    -60,     25}, 0, {     0,      0}, {0xa2, 0xbf, 0x36, 0x9e}}},
    {{{  -135,    -70,     23}, 0, {     0,      0}, {0xb2, 0xaa, 0x33, 0x9e}}},
    {{{  -155,      6,     33}, 0, {     0,      0}, {0x87, 0x0f, 0x22, 0x9e}}},
    {{{   200,    -60,     25}, 0, {     0,      0}, {0x5e, 0xbf, 0x36, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
    {{{   108,     20,    118}, 0, {     0,      0}, {0x57, 0x14, 0x5a, 0x9e}}},
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
    {{{   125,     99,     15}, 0, {     0,      0}, {0x65, 0x4c, 0x08, 0x9e}}},
    {{{   125,     99,     15}, 0, {     0,      0}, {0x65, 0x4c, 0x08, 0x9e}}},
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
    {{{   200,    -60,     25}, 0, {     0,      0}, {0x5e, 0xbf, 0x36, 0x9e}}},
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
    {{{   136,    -70,     23}, 0, {     0,      0}, {0x4e, 0xaa, 0x33, 0x9e}}},
};

// 0x0500BB50
static const Vtx boo_seg5_vertex_0500BB50[] = {
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
    {{{   200,    -60,     25}, 0, {     0,      0}, {0x5e, 0xbf, 0x36, 0x9e}}},
    {{{   136,    -70,     23}, 0, {     0,      0}, {0x4e, 0xaa, 0x33, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{   -43,    162,     15}, 0, {     0,      0}, {0xdf, 0x7a, 0xf8, 0x9e}}},
    {{{   -41,    130,    -88}, 0, {     0,      0}, {0xd4, 0x65, 0xc3, 0x9e}}},
    {{{  -124,     99,     15}, 0, {     0,      0}, {0x9b, 0x4c, 0x08, 0x9e}}},
    {{{   -57,    128,    108}, 0, {     0,      0}, {0xd3, 0x60, 0x45, 0x9e}}},
    {{{   -43,    162,     15}, 0, {     0,      0}, {0xdf, 0x7a, 0xf8, 0x9e}}},
    {{{  -124,     99,     15}, 0, {     0,      0}, {0x9b, 0x4c, 0x08, 0x9e}}},
    {{{   128,    -69,    -89}, 0, {     0,      0}, {0x64, 0xc6, 0xce, 0x9e}}},
    {{{    46,    -82,   -160}, 0, {     0,      0}, {0x2e, 0xba, 0xa2, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
};

// 0x0500BC40
static const Vtx boo_seg5_vertex_0500BC40[] = {
    {{{     0,     62,   -162}, 0, {     0,      0}, {0x00, 0x5a, 0xa8, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
    {{{     0,     24,   -213}, 0, {     0,      0}, {0x00, 0x22, 0x86, 0x9e}}},
    {{{     0,     24,   -213}, 0, {     0,      0}, {0x00, 0x22, 0x86, 0x9e}}},
    {{{   122,     59,    -94}, 0, {     0,      0}, {0x5f, 0x2e, 0xbb, 0x9e}}},
    {{{    46,    -82,   -160}, 0, {     0,      0}, {0x2e, 0xba, 0xa2, 0x9e}}},
    {{{     0,     62,   -162}, 0, {     0,      0}, {0x00, 0x5a, 0xa8, 0x9e}}},
    {{{     0,     24,   -213}, 0, {     0,      0}, {0x00, 0x22, 0x86, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{   -41,    130,    -88}, 0, {     0,      0}, {0xd4, 0x65, 0xc3, 0x9e}}},
    {{{     0,     62,   -162}, 0, {     0,      0}, {0x00, 0x5a, 0xa8, 0x9e}}},
    {{{     0,     24,   -213}, 0, {     0,      0}, {0x00, 0x22, 0x86, 0x9e}}},
    {{{    46,    -82,   -160}, 0, {     0,      0}, {0x2e, 0xba, 0xa2, 0x9e}}},
    {{{   -45,    -82,   -160}, 0, {     0,      0}, {0xc9, 0xc5, 0x9f, 0x9e}}},
};

// 0x0500BD30
static const Vtx boo_seg5_vertex_0500BD30[] = {
    {{{   -45,    -82,   -160}, 0, {     0,      0}, {0xc9, 0xc5, 0x9f, 0x9e}}},
    {{{    46,    -82,   -160}, 0, {     0,      0}, {0x2e, 0xba, 0xa2, 0x9e}}},
    {{{   -59,   -126,    -86}, 0, {     0,      0}, {0xe1, 0x90, 0xd0, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{   -45,    -82,   -160}, 0, {     0,      0}, {0xc9, 0xc5, 0x9f, 0x9e}}},
    {{{   -45,    -82,   -160}, 0, {     0,      0}, {0xc9, 0xc5, 0x9f, 0x9e}}},
    {{{   -59,   -126,    -86}, 0, {     0,      0}, {0xe1, 0x90, 0xd0, 0x9e}}},
    {{{  -127,    -69,    -89}, 0, {     0,      0}, {0x9c, 0xc6, 0xce, 0x9e}}},
    {{{     0,     24,   -213}, 0, {     0,      0}, {0x00, 0x22, 0x86, 0x9e}}},
    {{{   -45,    -82,   -160}, 0, {     0,      0}, {0xc9, 0xc5, 0x9f, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{   -41,    130,    -88}, 0, {     0,      0}, {0xd4, 0x65, 0xc3, 0x9e}}},
    {{{  -121,     59,    -94}, 0, {     0,      0}, {0xa1, 0x2e, 0xbb, 0x9e}}},
    {{{  -124,     99,     15}, 0, {     0,      0}, {0x9b, 0x4c, 0x08, 0x9e}}},
};

// 0x0500BE20
static const Vtx boo_seg5_vertex_0500BE20[] = {
    {{{   -57,    128,    108}, 0, {     0,      0}, {0xd3, 0x60, 0x45, 0x9e}}},
    {{{     0,     60,    166}, 0, {     0,      0}, {0x00, 0x26, 0x79, 0x9e}}},
    {{{    58,    128,    108}, 0, {     0,      0}, {0x35, 0x58, 0x49, 0x9e}}},
    {{{   108,     20,    118}, 0, {     0,      0}, {0x57, 0x14, 0x5a, 0x9e}}},
    {{{   125,     99,     15}, 0, {     0,      0}, {0x65, 0x4c, 0x08, 0x9e}}},
    {{{    58,    128,    108}, 0, {     0,      0}, {0x35, 0x58, 0x49, 0x9e}}},
    {{{   136,    -70,     23}, 0, {     0,      0}, {0x4e, 0xaa, 0x33, 0x9e}}},
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
    {{{    87,    -78,    123}, 0, {     0,      0}, {0x53, 0xc9, 0x4e, 0x9e}}},
    {{{   108,     20,    118}, 0, {     0,      0}, {0x57, 0x14, 0x5a, 0x9e}}},
    {{{    87,    -78,    123}, 0, {     0,      0}, {0x53, 0xc9, 0x4e, 0x9e}}},
    {{{   156,      6,     33}, 0, {     0,      0}, {0x79, 0x0f, 0x22, 0x9e}}},
};

// 0x0500BEE0 - 0x0500BF48
const Gfx boo_seg5_dl_0500BEE0[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, boo_seg5_texture_0500AB40),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPLight(&boo_seg5_lights_05009B28.l, 1),
    gsSPLight(&boo_seg5_lights_05009B28.a, 2),
    gsSPVertex(boo_seg5_vertex_0500B340, 12, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSPEndDisplayList(),
};

// 0x0500BF48 - 0x0500BFA0
const Gfx boo_seg5_dl_0500BF48[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, boo_seg5_texture_05009B40),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 64 * 32 - 1, CALC_DXT(64, G_IM_SIZ_16b_BYTES)),
    gsSPVertex(boo_seg5_vertex_0500B400, 12, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSPEndDisplayList(),
};

// 0x0500BFA0 - 0x0500C1B0
const Gfx boo_seg5_dl_0500BFA0[] = {
    gsSPVertex(boo_seg5_vertex_0500B4C0, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500B5B0, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500B6A0, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500B790, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500B880, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500B970, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500BA60, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500BB50, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500BC40, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500BD30, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(boo_seg5_vertex_0500BE20, 12, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSPEndDisplayList(),
};

// 0x0500C1B0 - 0x0500C250
const Gfx boo_seg5_dl_0500C1B0[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_BLENDRGBFADEA, G_CC_BLENDRGBFADEA),
    gsSPNumLights(NUMLIGHTS_1),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 5, G_TX_NOLOD, G_TX_CLAMP, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPDisplayList(boo_seg5_dl_0500BEE0),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 5, G_TX_NOLOD, G_TX_CLAMP, 6, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPDisplayList(boo_seg5_dl_0500BF48),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADEFADEA, G_CC_SHADEFADEA),
    gsSPDisplayList(boo_seg5_dl_0500BFA0),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsSPEndDisplayList(),
};
