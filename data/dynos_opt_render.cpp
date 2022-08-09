#include "dynos.cpp.h"
extern "C" {
#include "course_table.h"
#include "game/game_init.h"
#include "game/ingame_menu.h"
#include "game/segment2.h"
#include "pc/controller/controller_api.h"
#include "gfx_dimensions.h"
}

extern s32 sBindingState;

#define DYNOS_TEXT_DYNOS_MENU   { "DYNOS MENU",  NULL }
#define DYNOS_TEXT_A            { "([A]) >",     NULL }
#define DYNOS_TEXT_OPEN_LEFT    { "[Z] DynOS",   NULL }
#define DYNOS_TEXT_CLOSE_LEFT   { "[Z] Return",  NULL }
#define DYNOS_TEXT_OPTIONS_MENU { "OPTIONS",     NULL }
#define DYNOS_TEXT_DISABLED     { "Disabled",    NULL }
#define DYNOS_TEXT_ENABLED      { "Enabled",     NULL }
#define DYNOS_TEXT_NONE         { "NONE",        NULL }
#define DYNOS_TEXT_DOT_DOT_DOT  { "...",         NULL }
#define DYNOS_TEXT_OPEN_RIGHT   { "[R] Options", NULL }
#define DYNOS_TEXT_CLOSE_RIGHT  { "[R] Return",  NULL }

static void RenderString(const u8 *aStr64, s32 aX, s32 aY) {
    create_dl_translation_matrix(MENU_MTX_PUSH, aX, aY, 0);
    for (; *aStr64 != DIALOG_CHAR_TERMINATOR; ++aStr64) {
        if (*aStr64 != DIALOG_CHAR_SPACE) {
            void **fontLUT = (void **) segmented_to_virtual(main_font_lut);
            void *packedTexture = segmented_to_virtual(fontLUT[*aStr64]);
            gDPPipeSync(gDisplayListHead++);
            gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(packedTexture));
            gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
        }
        create_dl_translation_matrix(MENU_MTX_NOPUSH, DynOS_String_WidthChar64(*aStr64), 0, 0);
    }
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

static void PrintString(const Label& aLabel, s32 aX, s32 aY, u32 aFrontColorRGBA, u32 aBackColorRGBA, bool aAlignLeft) {
    const u8 *_Str64 = (aLabel.second ? aLabel.second : DynOS_String_Convert(aLabel.first.begin(), false));

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    if ((aBackColorRGBA & 0xFF) != 0) {
        gDPSetEnvColor(gDisplayListHead++, ((aBackColorRGBA >> 24) & 0xFF), ((aBackColorRGBA >> 16) & 0xFF), ((aBackColorRGBA >> 8) & 0xFF), ((aBackColorRGBA >> 0) & 0xFF));
        if (aAlignLeft) {
            RenderString(_Str64, GFX_DIMENSIONS_FROM_LEFT_EDGE(aX) + 1, aY - 1);
        } else {
            RenderString(_Str64, GFX_DIMENSIONS_FROM_RIGHT_EDGE(aX + DynOS_String_Width(_Str64) - 1), aY - 1);
        }
    }
    if ((aFrontColorRGBA & 0xFF) != 0) {
        gDPSetEnvColor(gDisplayListHead++, ((aFrontColorRGBA >> 24) & 0xFF), ((aFrontColorRGBA >> 16) & 0xFF), ((aFrontColorRGBA >> 8) & 0xFF), ((aFrontColorRGBA >> 0) & 0xFF));
        if (aAlignLeft) {
            RenderString(_Str64, GFX_DIMENSIONS_FROM_LEFT_EDGE(aX), aY);
        } else {
            RenderString(_Str64, GFX_DIMENSIONS_FROM_RIGHT_EDGE(aX + DynOS_String_Width(_Str64)), aY);
        }
    }
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

static void PrintBox(s32 aX, s32 aY, s32 aWidth, s32 aHeight, u32 aColorRGBA, bool aAlignLeft) {
    if ((aColorRGBA && 0xFF) != 0) {
        Mtx *_Matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (!_Matrix) return;
        if (aAlignLeft) {
            create_dl_translation_matrix(MENU_MTX_PUSH, GFX_DIMENSIONS_FROM_LEFT_EDGE(aX), aY + aHeight, 0);
        } else {
            create_dl_translation_matrix(MENU_MTX_PUSH, GFX_DIMENSIONS_FROM_RIGHT_EDGE(aX + aWidth), aY + aHeight, 0);
        }
        guScale(_Matrix, (f32) aWidth / 130.f, (f32) aHeight / 80.f, 1.f);
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(_Matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
        gDPSetEnvColor(gDisplayListHead++, ((aColorRGBA >> 24) & 0xFF), ((aColorRGBA >> 16) & 0xFF), ((aColorRGBA >> 8) & 0xFF), ((aColorRGBA >> 0) & 0xFF));
        gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
        gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    }
}

static const char *IntToString(const char *fmt, s32 x) {
    static char sBuffer[16];
    snprintf(sBuffer, 16, fmt, x);
    return sBuffer;
}

#define get_label(opt)      (opt->mLabel)
#define get_title(opt)      (opt->mTitle)
#define get_choice(opt)     (opt->mChoice.mChoices[*opt->mChoice.mIndex])
#define get_dec_number(n)   { "", DynOS_String_Convert(IntToString("%d", n), false) }
#define get_hex_number(n)   { "", DynOS_String_Convert(IntToString("%04X", n), false) }
#define get_level(opt)      { "", DynOS_Level_GetName(DynOS_Level_GetList()[*opt->mChoice.mIndex], true, true) }
#define get_star(opt)       { "", DynOS_Level_GetActName(DynOS_Level_GetList()[DynOS_Opt_GetValue("dynos_warp_level")], *opt->mChoice.mIndex + 1, true, true) }
#define get_param(opt)      { DynOS_Warp_GetParamName(DynOS_Level_GetList()[DynOS_Opt_GetValue("dynos_warp_level")], *opt->mChoice.mIndex), NULL }

static s32 GetCurrentOptionCount(DynosOption *aCurrentOpt) {
    s32 _Count = 0;
    while (aCurrentOpt->mPrev) { aCurrentOpt = aCurrentOpt->mPrev; }
    while (aCurrentOpt) { aCurrentOpt = aCurrentOpt->mNext; _Count++; }
    return _Count;
}

static s32 GetCurrentOptionIndex(DynosOption *aCurrentOpt) {
    s32 _Index = 0;
    while (aCurrentOpt->mPrev) { aCurrentOpt = aCurrentOpt->mPrev; _Index++; }
    return _Index;
}

#define PREV(opt) (opt == NULL ? NULL : opt->mPrev)
#define NEXT(opt) (opt == NULL ? NULL : opt->mNext)
static DynosOption **GetCurrentOptions(DynosOption *aCurrentOpt) {
    static DynosOption *sOptionList[13];

    sOptionList[6]  = aCurrentOpt;
    sOptionList[5]  = PREV(sOptionList[6]);
    sOptionList[4]  = PREV(sOptionList[5]);
    sOptionList[3]  = PREV(sOptionList[4]);
    sOptionList[2]  = PREV(sOptionList[3]);
    sOptionList[1]  = PREV(sOptionList[2]);
    sOptionList[0]  = PREV(sOptionList[1]);
    sOptionList[7]  = NEXT(sOptionList[6]);
    sOptionList[8]  = NEXT(sOptionList[7]);
    sOptionList[9]  = NEXT(sOptionList[8]);
    sOptionList[10] = NEXT(sOptionList[9]);
    sOptionList[11] = NEXT(sOptionList[10]);
    sOptionList[12] = NEXT(sOptionList[11]);

    s32 _StartIndex = 12, _EndIndex = 0;
    for (s32 i = 0; i != 13; ++i) {
        if (sOptionList[i] != NULL) {
            _StartIndex = MIN(_StartIndex, i);
            _EndIndex = MAX(_EndIndex, i);
        }
    }

    if (_EndIndex - _StartIndex < 7) {
        return &sOptionList[_StartIndex];
    }
    if (_EndIndex <= 9) {
        return &sOptionList[_EndIndex - 6];
    }
    if (_StartIndex >= 3) {
        return &sOptionList[_StartIndex];
    }
    return &sOptionList[3];
}
#undef PREV
#undef NEXT

#define COLOR_WHITE             0xFFFFFFFF
#define COLOR_BLACK             0x000000FF
#define COLOR_GRAY              0xA0A0A0FF
#define COLOR_DARK_GRAY         0x808080FF
#define COLOR_SELECT            0x80E0FFFF
#define COLOR_SELECT_BOX        0x00FFFF20
#define COLOR_ENABLED           0x20E020FF
#define COLOR_DISABLED          0xFF2020FF
#define OFFSET_FROM_LEFT_EDGE   (20.f * sqr(GFX_DIMENSIONS_ASPECT_RATIO))
#define OFFSET_FROM_RIGHT_EDGE  (20.f * sqr(GFX_DIMENSIONS_ASPECT_RATIO))
#define SCROLL_BAR_SIZE         ((s32) (45.f * GFX_DIMENSIONS_ASPECT_RATIO))

static void DynOS_Opt_DrawOption(DynosOption *aOpt, DynosOption *aCurrentOpt, s32 aY) {
    if (aOpt == NULL) {
        return;
    }

    // Selected box
    if (aOpt == aCurrentOpt) {
        u8 _Alpha = (u8) ((coss(gGlobalTimer * 0x800) + 1.f) * 0x20);
        PrintBox(OFFSET_FROM_LEFT_EDGE - 4, aY - 2, GFX_DIMENSIONS_FROM_RIGHT_EDGE(OFFSET_FROM_RIGHT_EDGE) - GFX_DIMENSIONS_FROM_LEFT_EDGE(OFFSET_FROM_LEFT_EDGE) + 8, 20, COLOR_SELECT_BOX + _Alpha, 1);
    }

    // Label
    if (aOpt == aCurrentOpt) {
        PrintString(get_label(aOpt), OFFSET_FROM_LEFT_EDGE, aY, COLOR_SELECT, COLOR_BLACK, 1);
    } else {
        PrintString(get_label(aOpt), OFFSET_FROM_LEFT_EDGE, aY, COLOR_WHITE, COLOR_BLACK, 1);
    }

    // Values
    switch (aOpt->mType) {
        case DOPT_TOGGLE: {
            if (*aOpt->mToggle.mTog) {
                PrintString(DYNOS_TEXT_ENABLED, OFFSET_FROM_RIGHT_EDGE, aY, COLOR_ENABLED, COLOR_BLACK, 0);
            } else {
                PrintString(DYNOS_TEXT_DISABLED, OFFSET_FROM_RIGHT_EDGE, aY, COLOR_DISABLED, COLOR_BLACK, 0);
            }
        } break;

        case DOPT_CHOICE: {
            PrintString(get_choice(aOpt), OFFSET_FROM_RIGHT_EDGE, aY, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, COLOR_BLACK, 0);
        } break;

        case DOPT_CHOICELEVEL: {
            PrintString(get_level(aOpt), OFFSET_FROM_RIGHT_EDGE, aY, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, COLOR_BLACK, 0);
        } break;

        case DOPT_CHOICEAREA: {
            s32 _Level = DynOS_Level_GetList()[DynOS_Opt_GetValue("dynos_warp_level")];
            s32 _Area = *aOpt->mChoice.mIndex + 1;
            const u8 *_Name = DynOS_Level_GetAreaName(_Level, _Area, true);
            if (DynOS_Level_GetWarpEntry(_Level, _Area)) {
                PrintString({ "", _Name }, OFFSET_FROM_RIGHT_EDGE, aY, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, COLOR_BLACK, 0);
            } else {
                PrintString({ "", _Name }, OFFSET_FROM_RIGHT_EDGE, aY, COLOR_GRAY, COLOR_BLACK, 0);
            }
        } break;

        case DOPT_CHOICESTAR: {
            s32 _Course = DynOS_Level_GetCourse(DynOS_Level_GetList()[DynOS_Opt_GetValue("dynos_warp_level")]);
            if (_Course >= COURSE_MIN && _Course <= COURSE_STAGES_MAX) {
                PrintString(get_star(aOpt), OFFSET_FROM_RIGHT_EDGE, aY, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, COLOR_BLACK, 0);
            }
        } break;

#ifndef DYNOS_COOP
        case DOPT_CHOICEPARAM: {
            PrintString(get_param(aOpt), OFFSET_FROM_RIGHT_EDGE, aY, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, COLOR_BLACK, 0);
        } break;
#endif

        case DOPT_SCROLL: {
            s32 _Width = (s32) (SCROLL_BAR_SIZE * (f32) (*aOpt->mScroll.mValue - aOpt->mScroll.mMin) / (f32) (aOpt->mScroll.mMax - aOpt->mScroll.mMin));
            PrintString(get_dec_number(*aOpt->mScroll.mValue), OFFSET_FROM_RIGHT_EDGE, aY, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, COLOR_BLACK, 0);
            PrintBox(OFFSET_FROM_RIGHT_EDGE + 28, aY + 4, SCROLL_BAR_SIZE + 2, 8, COLOR_DARK_GRAY, 0);
            PrintBox(OFFSET_FROM_RIGHT_EDGE + 29 + SCROLL_BAR_SIZE - _Width, aY + 5, _Width, 6, aOpt == aCurrentOpt ? COLOR_SELECT : COLOR_WHITE, 0);
        } break;

        case DOPT_BIND: {
            for (s32 i = 0; i != 3; ++i) {
                u32 _Bind = aOpt->mBind.mBinds[i];
                if (aOpt == aCurrentOpt && i == aOpt->mBind.mIndex) {
                    if (sBindingState != 0) {
                        PrintString(DYNOS_TEXT_DOT_DOT_DOT, OFFSET_FROM_RIGHT_EDGE + (2 - i) * 36, aY, COLOR_SELECT, COLOR_BLACK, 0);
                    } else if (_Bind == VK_INVALID) {
                        PrintString(DYNOS_TEXT_NONE, OFFSET_FROM_RIGHT_EDGE + (2 - i) * 36, aY, COLOR_SELECT, COLOR_BLACK, 0);
                    } else {
                        PrintString(get_hex_number(_Bind), OFFSET_FROM_RIGHT_EDGE + (2 - i) * 36, aY, COLOR_SELECT, COLOR_BLACK, 0);
                    }
                } else {
                    if (_Bind == VK_INVALID) {
                        PrintString(DYNOS_TEXT_NONE, OFFSET_FROM_RIGHT_EDGE + (2 - i) * 36, aY, COLOR_GRAY, COLOR_BLACK, 0);
                    } else {
                        PrintString(get_hex_number(_Bind), OFFSET_FROM_RIGHT_EDGE + (2 - i) * 36, aY, COLOR_WHITE, COLOR_BLACK, 0);
                    }
                }
            }
        } break;

        case DOPT_BUTTON: {
        } break;

        case DOPT_SUBMENU: {
            if (aOpt == aCurrentOpt) {
                PrintString(DYNOS_TEXT_A, OFFSET_FROM_RIGHT_EDGE, aY, COLOR_SELECT, COLOR_BLACK, 0);
            }
        } break;
    }
}

void DynOS_Opt_DrawMenu(DynosOption *aCurrentOption, DynosOption *aCurrentMenu, DynosOption *aOptionsMenu, DynosOption *aDynosMenu) {
    if (aCurrentMenu == NULL) {
        return;
    }

    // Colorful label
    Label _Title;
    if (aCurrentOption->mParent) {
        _Title = get_title(aCurrentOption->mParent);
    } else if (aCurrentMenu == aDynosMenu) {
        _Title = DYNOS_TEXT_DYNOS_MENU;
    } else if (aCurrentMenu == aOptionsMenu) {
        _Title = DYNOS_TEXT_OPTIONS_MENU;
    }
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    if (!_Title.second) _Title.second = DynOS_String_Convert(_Title.first.begin(), false);
    print_hud_lut_string(HUD_LUT_GLOBAL, (SCREEN_WIDTH / 2 - DynOS_String_Length(_Title.second) * 6), 40, _Title.second);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    // Display options
    DynosOption **_Options = GetCurrentOptions(aCurrentOption);
    for (s32 i = 0; i != 7; ++i) {
        DynOS_Opt_DrawOption(_Options[i], aCurrentOption, 156 - 20 * i);
    }

    // Scroll bar
    s32 _OptCount = GetCurrentOptionCount(aCurrentOption);
    s32 _OptIndex = GetCurrentOptionIndex(aCurrentOption);
    if (_OptCount > 7) {
        s32 _Height = (s32) (134.f * sqrtf(1.f / (_OptCount - 6)));
        s32 _Y = 37 + (134 - _Height) * (1.f - MAX(0.f, MIN(1.f, (f32)(_OptIndex - 3) / (f32)(_OptCount - 6))));
        PrintBox(OFFSET_FROM_RIGHT_EDGE - 16, 36, 8, 136, COLOR_DARK_GRAY, 0);
        PrintBox(OFFSET_FROM_RIGHT_EDGE - 15, _Y, 6, _Height, COLOR_WHITE, 0);
    }
}

#define PROMPT_OFFSET (56.25f * GFX_DIMENSIONS_ASPECT_RATIO)
void DynOS_Opt_DrawPrompt(DynosOption *aCurrentMenu, DynosOption *aOptionsMenu, DynosOption *aDynosMenu) {
    if (aCurrentMenu == aOptionsMenu) {
        PrintString(DYNOS_TEXT_OPEN_LEFT,   PROMPT_OFFSET, 212, COLOR_WHITE, COLOR_BLACK, 1);
        PrintString(DYNOS_TEXT_CLOSE_RIGHT, PROMPT_OFFSET, 212, COLOR_WHITE, COLOR_BLACK, 0);
    } else if (aCurrentMenu == aDynosMenu) {
        PrintString(DYNOS_TEXT_CLOSE_LEFT,  PROMPT_OFFSET, 212, COLOR_WHITE, COLOR_BLACK, 1);
        PrintString(DYNOS_TEXT_OPEN_RIGHT,  PROMPT_OFFSET, 212, COLOR_WHITE, COLOR_BLACK, 0);
    } else {
        PrintString(DYNOS_TEXT_OPEN_LEFT,   PROMPT_OFFSET, 212, COLOR_WHITE, COLOR_BLACK, 1);
        PrintString(DYNOS_TEXT_OPEN_RIGHT,  PROMPT_OFFSET, 212, COLOR_WHITE, COLOR_BLACK, 0);
    }
}
