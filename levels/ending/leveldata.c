#include <ultra64.h>
#include "sm64.h"
#include "surface_terrains.h"
#include "moving_texture_macros.h"
#include "level_misc_macros.h"
#include "macro_preset_names.h"
#include "special_preset_names.h"
#include "textures.h"

#include "make_const_nonconst.h"

const Gfx dl_cake_end_screen[] = {
    gsDPPipeSync(),
    //gsDPSetCombineMode(G_CC_DECALRGB, G_CC_DECALRGB),
    //gsDPSetRenderMode(G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2),
    //gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),

    //gsDPLoadTextureBlock(cake_end_texture_0, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025800, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_1, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025840, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_2, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025880, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_3, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_070258C0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_4, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025900, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_5, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025940, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_6, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025980, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_7, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_070259C0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_8, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025A00, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_9, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025A40, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_10, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025A80, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_11, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025AC0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_12, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025B00, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_13, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025B40, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_14, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025B80, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_15, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025BC0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_16, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025C00, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_17, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025C40, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_18, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025C80, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_19, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025CC0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_20, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025D00, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_21, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025D40, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_22, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025D80, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_23, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025DC0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_24, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025E00, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_25, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025E40, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_26, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025E80, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_27, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025EC0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_28, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025F00, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_29, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025F40, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_30, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025F80, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_31, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07025FC0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_32, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026000, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_33, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026040, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_34, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026080, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_35, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_070260C0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_36, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026100, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_37, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026140, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_38, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026180, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_39, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_070261C0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_40, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026200, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_41, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026240, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_42, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026280, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_43, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_070262C0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_44, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026300, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_45, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026340, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_46, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_07026380, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPLoadTextureBlock(cake_end_texture_47, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 20, 0, G_TX_CLAMP, G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD),
    //gsSPVertex(cake_end_vertex_070263C0, 4, 0),
    //gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),

    //gsDPPipeSync(),
    //gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    //gsSPSetGeometryMode(G_LIGHTING),
    //gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    //gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsSPEndDisplayList(),
};
