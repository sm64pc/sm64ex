#include "moon-gfx.h"
#include "game/ingame_menu.h"
#include "game/game_init.h"
#include "game/segment2.h"

f32 moon_get_text_width(u8* text, float scale, u8 colored) { 
    f32 size = 0;
    s32 strPos = 0;

    while (text[strPos] != (colored ? GLOBAR_CHAR_TERMINATOR : DIALOG_CHAR_TERMINATOR)) {
        if(colored)
            size += (text[strPos] == GLOBAL_CHAR_SPACE ? 8.0 : 12.0) * scale;
        else
            size += (f32)(gDialogCharWidths[text[strPos]]) * scale;            
        strPos++;
    }

    return size;
}

void moon_draw_colored_text(f32 x, f32 y, const u8 *str, float scale) {    
    void **hudLUT2 = segmented_to_virtual(main_hud_lut);
    u32 xStride = round(12 * scale);
    s32 strPos  = 0;
    s32 w       = round(16 * scale);    

    while (str[strPos] != GLOBAR_CHAR_TERMINATOR) {
        if (str[strPos] == GLOBAL_CHAR_SPACE) {
            x += round(8 * scale);
        } else {
            moon_draw_texture(x, y, w, w, hudLUT2[str[strPos]]);
            x += xStride;
        }
        strPos++;
    }
}

void moon_draw_text(f32 x, f32 y, const u8 *str, float scale) {
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in EU
    s32 strPos = 0;
    u8 lineNum = 1;
    
    Mtx *_Matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
    if (!_Matrix) return;    
    guScale(_Matrix, scale, scale - 0.1f, 1.f);
    create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0.0f);
    gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(_Matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        switch (str[strPos]) {
            case DIALOG_CHAR_DAKUTEN:
                mark = DIALOG_MARK_DAKUTEN;
                break;
            case DIALOG_CHAR_PERIOD_OR_HANDAKUTEN:
                mark = DIALOG_MARK_HANDAKUTEN;
                break;
            case DIALOG_CHAR_NEWLINE:
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                create_dl_translation_matrix(MENU_MTX_PUSH, x, y - (lineNum * MAX_STRING_WIDTH), 0.0f);
                lineNum++;
                break;
            case DIALOG_CHAR_PERIOD:
                create_dl_translation_matrix(MENU_MTX_PUSH, -2.0f, -5.0f, 0.0f);
                render_generic_char(DIALOG_CHAR_PERIOD_OR_HANDAKUTEN);
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                break;
            case DIALOG_CHAR_SLASH:
                create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * 2), 0.0f, 0.0f);
                break;
            case DIALOG_CHAR_MULTI_THE:
                render_multi_text_string(STRING_THE);
                break;
            case DIALOG_CHAR_MULTI_YOU:
                render_multi_text_string(STRING_YOU);
                break;
            case DIALOG_CHAR_SPACE:
                create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE]), 0.0f, 0.0f);
                break; // ? needed to match
            default:
                render_generic_char(str[strPos]);
                if (mark != DIALOG_MARK_NONE) {
                    //create_dl_translation_matrix(MENU_MTX_PUSH, 5.0f, 5.0f, 0.0f);
                    guScale(gDisplayListHead++, scale, scale, scale);
                    render_generic_char(mark + 0xEF);
                    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                    mark = DIALOG_MARK_NONE;
                }
                create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[str[strPos]]), 0.0f, 0.0f);
        }

        strPos++;
    }
    
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.0f, 1.0f, 1.0f);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void moon_draw_texture(s32 x, s32 y, u32 w, u32 h, u8 *texture) {
    gSPDisplayList(gDisplayListHead++, dl_hud_img_begin);    
    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD);
    gDPTileSync(gDisplayListHead++);
    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 2, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR, 3, G_TX_NOLOD, G_TX_NOMIRROR, 3, G_TX_NOLOD);
    gDPSetTileSize(gDisplayListHead++, G_TX_RENDERTILE, 0, 0, w << G_TEXTURE_IMAGE_FRAC, h << G_TEXTURE_IMAGE_FRAC);
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_32b, 1, texture);
    gDPLoadSync(gDisplayListHead++);
    gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, w * h - 1, CALC_DXT(w, G_IM_SIZ_32b_BYTES));
    gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + w) << 2, (y + h) << 2, G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10);
    gSPDisplayList(gDisplayListHead++, dl_hud_img_end);
}

void moon_draw_rectangle(s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
    gDPSetFillColor(gDisplayListHead++, GPACK_RGBA5551(r, g, b, 255));
    gDPFillRectangle(gDisplayListHead++, x1, y1, x2 + 1, y2 + 1);
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
}