#ifdef EXT_OPTIONS_MENU

#include "sm64.h"
#include "include/text_strings.h"
#include "engine/math_util.h"
#include "audio/external.h"
#include "game/camera.h"
#include "game/level_update.h"
#include "game/print.h"
#include "game/segment2.h"
#include "game/save_file.h"
#include "game/bettercamera.h"
#include "game/mario_misc.h"
#include "game/game_init.h"
#include "game/ingame_menu.h"
#include "game/options_menu.h"
#include "pc/configfile.h"
#include "pc/controller/controller_api.h"

#include <stdbool.h>

u8 optmenu_open = 0;

static u8 optmenu_binding = 0;
static u8 optmenu_bind_idx = 0;

// How to add stuff:
// strings: add them to include/text_strings.h.in
//          and to menuStr[] / opts*Str[]
// options: add them to the relevant options list
// menus:   add a new submenu definition and a new
//          option to the optsMain list

static const u8 toggleStr[][64] = {
    { TEXT_OPT_DISABLED },
    { TEXT_OPT_ENABLED },
};

static const u8 menuStr[][64] = {
    { TEXT_OPT_HIGHLIGHT },
    { TEXT_OPT_BUTTON1 },
    { TEXT_OPT_BUTTON2 },
    { TEXT_OPT_OPTIONS },
    { TEXT_OPT_CAMERA },
    { TEXT_OPT_CONTROLS },
};

static const u8 optsCameraStr[][64] = {
    { TEXT_OPT_CAMX },
    { TEXT_OPT_CAMY },
    { TEXT_OPT_INVERTX },
    { TEXT_OPT_INVERTY },
    { TEXT_OPT_CAMC },
    { TEXT_OPT_CAMP },
    { TEXT_OPT_ANALOGUE },
    { TEXT_OPT_MOUSE },
};

static const u8 bindStr[][64] = {
    { TEXT_OPT_UNBOUND },
    { TEXT_OPT_PRESSKEY },
    { TEXT_BIND_A },
    { TEXT_BIND_B },
    { TEXT_BIND_START },
    { TEXT_BIND_L },
    { TEXT_BIND_R },
    { TEXT_BIND_Z },
    { TEXT_BIND_C_UP },
    { TEXT_BIND_C_DOWN },
    { TEXT_BIND_C_LEFT },
    { TEXT_BIND_C_RIGHT },
    { TEXT_BIND_UP },
    { TEXT_BIND_DOWN },
    { TEXT_BIND_LEFT },
    { TEXT_BIND_RIGHT },
};

enum OptType {
    OPT_INVALID = 0,
    OPT_TOGGLE,
    OPT_CHOICE,
    OPT_SCROLL,
    OPT_SUBMENU,
    OPT_BIND,
};

struct SubMenu;

struct Option {
    enum OptType type;
    const u8 *label;
    union {
        u32 *uval;
        bool *bval;
    };
    union {
        struct {
            const u8 **choices;
            u32 numChoices;
        };
        struct {
            u32 scrMin;
            u32 scrMax;
            u32 scrStep;
        };
        struct SubMenu *nextMenu;
    };
};

struct SubMenu {
    struct SubMenu *prev; // this is set at runtime to avoid needless complication
    const u8 *label;
    struct Option *opts;
    s32 numOpts;
    s32 select;
    s32 scroll;
};

/* submenu option lists */

static struct Option optsCamera[] = {
    { .type = OPT_TOGGLE, .label = optsCameraStr[6], .bval = &configEnableCamera,                                          },
    { .type = OPT_TOGGLE, .label = optsCameraStr[7], .bval = &configCameraMouse,                                           },
    { .type = OPT_TOGGLE, .label = optsCameraStr[2], .bval = &configCameraInvertX,                                         },
    { .type = OPT_TOGGLE, .label = optsCameraStr[3], .bval = &configCameraInvertY,                                         },
    { .type = OPT_SCROLL, .label = optsCameraStr[0], .uval = &configCameraXSens, .scrMin = 10, .scrMax = 250, .scrStep = 1 },
    { .type = OPT_SCROLL, .label = optsCameraStr[1], .uval = &configCameraYSens, .scrMin = 10, .scrMax = 250, .scrStep = 1 },
    { .type = OPT_SCROLL, .label = optsCameraStr[4], .uval = &configCameraAggr,  .scrMin = 0,  .scrMax = 100, .scrStep = 1 },
    { .type = OPT_SCROLL, .label = optsCameraStr[5], .uval = &configCameraPan,   .scrMin = 0,  .scrMax = 100, .scrStep = 1 },
};

static struct Option optsControls[] = {
    { .type = OPT_BIND, .label = bindStr[2],  .uval = configKeyA,          },
    { .type = OPT_BIND, .label = bindStr[3],  .uval = configKeyB,          },
    { .type = OPT_BIND, .label = bindStr[4],  .uval = configKeyStart,      },
    { .type = OPT_BIND, .label = bindStr[5],  .uval = configKeyL,          },
    { .type = OPT_BIND, .label = bindStr[6],  .uval = configKeyR,          },
    { .type = OPT_BIND, .label = bindStr[7],  .uval = configKeyZ,          },
    { .type = OPT_BIND, .label = bindStr[8],  .uval = configKeyCUp,        },
    { .type = OPT_BIND, .label = bindStr[9],  .uval = configKeyCDown,      },
    { .type = OPT_BIND, .label = bindStr[10], .uval = configKeyCLeft,      },
    { .type = OPT_BIND, .label = bindStr[11], .uval = configKeyCRight,     },
    { .type = OPT_BIND, .label = bindStr[12], .uval = configKeyStickUp,    },
    { .type = OPT_BIND, .label = bindStr[13], .uval = configKeyStickDown,  },
    { .type = OPT_BIND, .label = bindStr[14], .uval = configKeyStickLeft,  },
    { .type = OPT_BIND, .label = bindStr[15], .uval = configKeyStickRight, },
};

/* submenu definitions */

static struct SubMenu menuCamera = {
    .label = menuStr[4],
    .opts = optsCamera,
    .numOpts = sizeof(optsCamera) / sizeof(optsCamera[0]),
};

static struct SubMenu menuControls = {
    .label = menuStr[5],
    .opts = optsControls,
    .numOpts = sizeof(optsControls) / sizeof(optsControls[0]),
};

/* main options menu definition */

static struct Option optsMain[] = {
    { .type = OPT_SUBMENU, .label = menuStr[4], .nextMenu = &menuCamera, },
    { .type = OPT_SUBMENU, .label = menuStr[5], .nextMenu = &menuControls, },
};

static struct SubMenu menuMain = {
    .label = menuStr[3],
    .opts = optsMain,
    .numOpts = sizeof(optsMain) / sizeof(optsMain[0]),
};

/* implementation */

static s32 optmenu_option_timer = 0;
static u8 optmenu_hold_count = 0;

static struct SubMenu *currentMenu = &menuMain;

static inline s32 wrap_add(s32 a, const s32 b, const s32 min, const s32 max) {
    a += b;
    if (a < min) a = max - (min - a) + 1;
    else if (a > max) a = min + (a - max) - 1;
    return a;
}

static void uint_to_hex(u32 num, u8 *dst) {
    u8 places = 4;
    while (places--) {
        const u32 digit = num & 0xF;
        dst[places] = digit;
        num >>= 4;
    }
    dst[4] = DIALOG_CHAR_TERMINATOR;
}

//Displays a box.
static void optmenu_draw_box(s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
    gDPSetFillColor(gDisplayListHead++, GPACK_RGBA5551(r, g, b, 255));
    gDPFillRectangle(gDisplayListHead++, x1, y1, x2 - 1, y2 - 1);
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
}

static void optmenu_draw_text(s16 x, s16 y, const u8 *str, u8 col) {
    const u8 textX = get_str_x_pos_from_center(x, (u8*)str, 10.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    print_generic_string(textX+1, y-1, str);
    if (col == 0) {
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else {
        gDPSetEnvColor(gDisplayListHead++, 255, 32, 32, 255);
    }
    print_generic_string(textX, y, str);
}

static void optmenu_draw_opt(const struct Option *opt, s16 x, s16 y, u8 sel) {
    u8 buf[32] = { 0 };

    if (opt->type == OPT_SUBMENU)
        y -= 6;

    optmenu_draw_text(x, y, opt->label, sel);

    switch (opt->type) {
        case OPT_TOGGLE:
            optmenu_draw_text(x, y-13, toggleStr[(int)*opt->bval], sel);
            break;

        case OPT_CHOICE:
            optmenu_draw_text(x, y-13, opt->choices[*opt->uval], sel);
            break;

        case OPT_SCROLL:
            int_to_str(*opt->uval, buf);
            optmenu_draw_text(x, y-13, buf, sel);
            break;

        case OPT_BIND:
            x = 112;
            for (u8 i = 0; i < MAX_BINDS; ++i, x += 48) {
                const u8 white = (sel && (optmenu_bind_idx == i));
                // TODO: button names
                if (opt->uval[i] == VK_INVALID) {
                    if (optmenu_binding && white)
                        optmenu_draw_text(x, y-13, bindStr[1], 1);
                    else
                        optmenu_draw_text(x, y-13, bindStr[0], white);
                } else {
                    uint_to_hex(opt->uval[i], buf);
                    optmenu_draw_text(x, y-13, buf, white);
                }
            }
            break;

        default: break;
    };
}

static void optmenu_opt_change(struct Option *opt, s32 val) {
    switch (opt->type) {
        case OPT_TOGGLE:
            *opt->bval = !*opt->bval;
            break;

        case OPT_CHOICE:
            *opt->uval = wrap_add(*opt->uval, val, 0, opt->numChoices - 1);
            break;

        case OPT_SCROLL:
            *opt->uval = wrap_add(*opt->uval, val, opt->scrMin, opt->scrMax);
            break;

        case OPT_SUBMENU:
            opt->nextMenu->prev = currentMenu;
            currentMenu = opt->nextMenu;
            break;

        case OPT_BIND:
            if (val == 0xFF) {
                // clear the bind
                opt->uval[optmenu_bind_idx] = VK_INVALID;
            } else if (val == 0) {
                opt->uval[optmenu_bind_idx] = VK_INVALID;
                optmenu_binding = 1;
                controller_get_raw_key(); // clear the last key, which is probably A
            } else {
                optmenu_bind_idx = wrap_add(optmenu_bind_idx, val, 0, MAX_BINDS - 1);
            }
            break;

        default: break;
    };
}

static inline s16 get_hudstr_centered_x(const s16 sx, const u8 *str) {
    const u8 *chr = str;
    s16 len = 0;
    while (*chr != GLOBAR_CHAR_TERMINATOR) ++chr, ++len;
    return sx - len * 6; // stride is 12
}

//Options menu
void optmenu_draw(void) {
    s16 scroll;
    s16 scrollpos;

    const s16 labelX = get_hudstr_centered_x(160, currentMenu->label);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, labelX, 40, currentMenu->label);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (currentMenu->numOpts > 4) {
        optmenu_draw_box(272, 90, 280, 208, 0x80, 0x80, 0x80);
        scrollpos = 54 * ((f32)currentMenu->scroll / (currentMenu->numOpts-4));
        optmenu_draw_box(272, 90+scrollpos, 280, 154+scrollpos, 0xFF, 0xFF, 0xFF);
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 80, SCREEN_WIDTH, SCREEN_HEIGHT);
    for (u8 i = 0; i < currentMenu->numOpts; i++) {
        scroll = 140 - 32 * i + currentMenu->scroll * 32;
        // FIXME: just start from the first visible option bruh
        if (scroll <= 140 && scroll > 32)
            optmenu_draw_opt(&currentMenu->opts[i], 160, scroll, (currentMenu->select == i));
    }

    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    print_hud_lut_string(HUD_LUT_GLOBAL, 80, 90 + (32 * (currentMenu->select - currentMenu->scroll)), menuStr[0]);
    print_hud_lut_string(HUD_LUT_GLOBAL, 224, 90 + (32 * (currentMenu->select - currentMenu->scroll)), menuStr[0]);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

//This has been separated for interesting reasons. Don't question it.
void optmenu_draw_prompt(void) {
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    optmenu_draw_text(278, 212, menuStr[1 + optmenu_open], 0);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

void optmenu_toggle(void) {
    if (optmenu_open == 0) {
        #ifndef nosound
        play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
        #endif
        currentMenu = &menuMain;
        optmenu_open = 1;
    } else {
        #ifndef nosound
        play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gDefaultSoundArgs);
        #endif
        optmenu_open = 0;
        newcam_init_settings(); // load bettercam settings from config vars
        controller_reconfigure(); // rebind using new config values
        configfile_save(CONFIG_FILE);
    }
}

void optmenu_check_buttons(void) {
    if (optmenu_binding) {
        u32 key = controller_get_raw_key();
        if (key != VK_INVALID) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            currentMenu->opts[currentMenu->select].uval[optmenu_bind_idx] = key;
            optmenu_binding = 0;
            optmenu_option_timer = 12;
        }
        return;
    }

    if (gPlayer1Controller->buttonPressed & R_TRIG)
        optmenu_toggle();

    if (!optmenu_open) return;

    u8 allowInput = 0;

    optmenu_option_timer--;
    if (optmenu_option_timer <= 0) {
        if (optmenu_hold_count == 0) {
            optmenu_hold_count++;
            optmenu_option_timer = 10;
        } else {
            optmenu_option_timer = 3;
        }
        allowInput = 1;
    }

    if (ABS(gPlayer1Controller->stickY) > 60) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif

            if (gPlayer1Controller->stickY >= 60) {
                currentMenu->select--;
                if (currentMenu->select < 0)
                    currentMenu->select = currentMenu->numOpts-1;
            } else {
                currentMenu->select++;
                if (currentMenu->select >= currentMenu->numOpts)
                    currentMenu->select = 0;
            }

            if (currentMenu->select < currentMenu->scroll)
                currentMenu->scroll = currentMenu->select;
            else if (currentMenu->select > currentMenu->scroll + 3)
                currentMenu->scroll = currentMenu->select - 3;
        }
    } else if (ABS(gPlayer1Controller->stickX) > 60) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            if (gPlayer1Controller->stickX >= 60)
                optmenu_opt_change(&currentMenu->opts[currentMenu->select], 1);
            else
                optmenu_opt_change(&currentMenu->opts[currentMenu->select], -1);
        }
    } else if (gPlayer1Controller->buttonPressed & A_BUTTON) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            optmenu_opt_change(&currentMenu->opts[currentMenu->select], 0);
        }
    } else if (gPlayer1Controller->buttonPressed & B_BUTTON) {
        if (allowInput) {
            if (currentMenu->prev) {
                #ifndef nosound
                play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
                #endif
                currentMenu = currentMenu->prev;
            } else {
                // can't go back, exit the menu altogether
                optmenu_toggle();
            }
        }
    } else if (gPlayer1Controller->buttonPressed & Z_TRIG) {
        // HACK: clear binds with Z
        if (allowInput && currentMenu->opts[currentMenu->select].type == OPT_BIND)
            optmenu_opt_change(&currentMenu->opts[currentMenu->select], 0xFF);
    } else if (gPlayer1Controller->buttonPressed & START_BUTTON) {
        if (allowInput) optmenu_toggle();
    } else {
        optmenu_hold_count = 0;
        optmenu_option_timer = 0;
    }
}

#endif // EXT_OPTIONS_MENU
