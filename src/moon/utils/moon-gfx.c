#include "moon-gfx.h"
#include "game/ingame_menu.h"
#include "game/game_init.h"

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

void moon_draw_text(s16 x, s16 y, const u8 *str, float scale) {
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in EU
    s32 strPos = 0;
    u8 lineNum = 1;

    Mtx *_Matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
    if (!_Matrix) return;
    create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0.0f);
    guScale(_Matrix, scale, scale, 1.f);
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
                    create_dl_translation_matrix(MENU_MTX_PUSH, 5.0f, 5.0f, 0.0f);
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