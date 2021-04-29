#ifdef EXT_OPTIONS_MENU

#include "sm64.h"
#include "engine/math_util.h"
#include "audio/external.h"
#include "game/camera.h"
#include "game/level_update.h"
#include "game/print.h"
#include "game/segment2.h"
#include "game/save_file.h"
#include "game/hud.h"
#ifdef BETTERCAMERA
#include "game/bettercamera.h"
#endif
#include "game/mario_misc.h"
#include "game/game_init.h"
#include "game/ingame_menu.h"
#include "game/options_menu.h"
#include "pc/pc_main.h"
#include "pc/cliopts.h"
#include "pc/cheats.h"
#include "pc/configfile.h"
#include "pc/controller/controller_api.h"
#include <stdbool.h>
#include "../../include/libc/stdlib.h"

#include "text/txtconv.h"
#include "text/text-loader.h"
#include "gfx_dimensions.h"
#include "gfx_dimensions.h"
#include "moon/utils/moon-gfx.h"
#include "moon/utils/moon-math.h"

u8 optmenu_open = 0;

static u8 optmenu_binding = 0;
static u8 optmenu_bind_idx = 0;

/* Keeps track of how many times the user has pressed L while in the options menu, so cheats can be unlocked */
static s32 l_counter = 0;

// How to add stuff:
// strings: add them to include/text_strings.h.in
//          and to menuStr[] / opts*Str[]
// options: add them to the relevant options list
// menus:   add a new submenu definition and a new
//          option to the optsMain list

static const u8 toggleStr[][20] = {
    "TEXT_OPT_DISABLED",
    "TEXT_OPT_ENABLED"
};

static const u8 menuStr[][32] = {
    "TEXT_OPT_HIGHLIGHT",
    "TEXT_OPT_BUTTON1",
    "TEXT_OPT_BUTTON2",
    "TEXT_OPT_OPTIONS",
    "TEXT_OPT_CAMERA",
    "TEXT_OPT_CONTROLS",
    "TEXT_OPT_VIDEO",
    "TEXT_OPT_AUDIO",
    "TEXT_EXIT_GAME",
    "TEXT_OPT_CHEATS",
    "TEXT_OPT_GAME"
};

static const u8 optsCameraStr[][32] = {
    "TEXT_OPT_CAMX",
    "TEXT_OPT_CAMY",
    "TEXT_OPT_INVERTX",
    "TEXT_OPT_INVERTY",
    "TEXT_OPT_CAMC",
    "TEXT_OPT_CAMP",
    "TEXT_OPT_ANALOGUE",
    "TEXT_OPT_MOUSE",
    "TEXT_OPT_CAMD",
    "TEXT_OPT_CAMON"
};

static const u8 optsGameStr[][32] = {
    "TEXT_OPT_LANGUAGE",
    "TEXT_OPT_PRECACHE",
    "TEXT_OPT_SWITCH_HUD"
};

static const u8 optsVideoStr[][32] = {
    "TEXT_OPT_FSCREEN",
    "TEXT_OPT_TEXFILTER",
    "TEXT_OPT_NEAREST",
    "TEXT_OPT_LINEAR",
    "TEXT_OPT_RESETWND",
    "TEXT_OPT_VSYNC",
    "TEXT_OPT_AUTO",
    "TEXT_OPT_HUD",
    "TEXT_OPT_THREEPT",
    "TEXT_OPT_APPLY"
};

static const u8 optsAudioStr[][32] = {
    "TEXT_OPT_MVOLUME",
    "TEXT_OPT_MUSVOLUME",
    "TEXT_OPT_SFXVOLUME",
    "TEXT_OPT_ENVVOLUME"
};

static const u8 optsCheatsStr[][64] = {
    "TEXT_OPT_CHEAT1",
    "TEXT_OPT_CHEAT2",
    "TEXT_OPT_CHEAT3",
    "TEXT_OPT_CHEAT4",
    "TEXT_OPT_CHEAT5",
    "TEXT_OPT_CHEAT6",
    "TEXT_OPT_CHEAT7",
    "TEXT_OPT_CHEAT8",
    "TEXT_OPT_CHEAT9"
};

static const u8 bindStr[][32] = {
    "TEXT_OPT_UNBOUND",
    "TEXT_OPT_PRESSKEY",
    "TEXT_BIND_A",
    "TEXT_BIND_B",
    "TEXT_BIND_START",
    "TEXT_BIND_L",
    "TEXT_BIND_R",
    "TEXT_BIND_Z",
    "TEXT_BIND_C_UP",
    "TEXT_BIND_C_DOWN",
    "TEXT_BIND_C_LEFT",
    "TEXT_BIND_C_RIGHT",
    "TEXT_BIND_UP",
    "TEXT_BIND_DOWN",
    "TEXT_BIND_LEFT",
    "TEXT_BIND_RIGHT",
    "TEXT_OPT_DEADZONE",
    "TEXT_OPT_RUMBLE"
};

static const u8 *filterChoices[] = {
    optsVideoStr[2],
    optsVideoStr[3],
    optsVideoStr[8],
};

static const u8 *vsyncChoices[] = {
    toggleStr[0],
    toggleStr[1],
    optsVideoStr[6],
};

enum OptType {
    OPT_INVALID = 0,
    OPT_TOGGLE,
    OPT_CHOICE,
    OPT_SCROLL,
    OPT_SUBMENU,
    OPT_BIND,
    OPT_BUTTON,
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
        void (*actionFn)(struct Option *, s32);
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

/* helper macros */

#define DEF_OPT_TOGGLE(lbl, bv) \
    { .type = OPT_TOGGLE, .label = lbl, .bval = bv }
#define DEF_OPT_SCROLL(lbl, uv, min, max, st) \
    { .type = OPT_SCROLL, .label = lbl, .uval = uv, .scrMin = min, .scrMax = max, .scrStep = st }
#define DEF_OPT_CHOICE(lbl, uv, ch) \
    { .type = OPT_CHOICE, .label = lbl, .uval = uv, .choices = ch, .numChoices = sizeof(ch) / sizeof(ch[0]) }
#define DEF_OPT_SUBMENU(lbl, nm) \
    { .type = OPT_SUBMENU, .label = lbl, .nextMenu = nm }
#define DEF_OPT_BIND(lbl, uv) \
    { .type = OPT_BIND, .label = lbl, .uval = uv }
#define DEF_OPT_BUTTON(lbl, act) \
    { .type = OPT_BUTTON, .label = lbl, .actionFn = act }
#define DEF_SUBMENU(lbl, opt) \
    { .label = lbl, .opts = opt, .numOpts = sizeof(opt) / sizeof(opt[0]) }

/* button action functions */

static void optmenu_act_exit(UNUSED struct Option *self, s32 arg) {
    if (!arg) game_exit(); // only exit on A press and not directions
}

static void optvideo_reset_window(UNUSED struct Option *self, s32 arg) {
    if (!arg) {
        // Restrict reset to A press and not directions
        configWindow.reset = true;
        configWindow.settings_changed = true;
    }
}

static void optvideo_apply(UNUSED struct Option *self, s32 arg) {
    if (!arg) configWindow.settings_changed = true;
}

/* submenu option lists */

#ifdef BETTERCAMERA
static struct Option optsCamera[] = {
    DEF_OPT_TOGGLE( optsCameraStr[9], &configEnableCamera ),
    DEF_OPT_TOGGLE( optsCameraStr[6], &configCameraAnalog ),
    DEF_OPT_TOGGLE( optsCameraStr[7], &configCameraMouse ),
    DEF_OPT_TOGGLE( optsCameraStr[2], &configCameraInvertX ),
    DEF_OPT_TOGGLE( optsCameraStr[3], &configCameraInvertY ),
    DEF_OPT_SCROLL( optsCameraStr[0], &configCameraXSens, 1, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[1], &configCameraYSens, 1, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[4], &configCameraAggr, 0, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[5], &configCameraPan, 0, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[8], &configCameraDegrade, 0, 100, 1 ),
};
#endif

static struct Option optsControls[] = {
    DEF_OPT_BIND( bindStr[ 2], configKeyA ),
    DEF_OPT_BIND( bindStr[ 3], configKeyB ),
    DEF_OPT_BIND( bindStr[ 4], configKeyStart ),
    DEF_OPT_BIND( bindStr[ 5], configKeyL ),
    DEF_OPT_BIND( bindStr[ 6], configKeyR ),
    DEF_OPT_BIND( bindStr[ 7], configKeyZ ),
    DEF_OPT_BIND( bindStr[ 8], configKeyCUp ),
    DEF_OPT_BIND( bindStr[ 9], configKeyCDown ),
    DEF_OPT_BIND( bindStr[10], configKeyCLeft ),
    DEF_OPT_BIND( bindStr[11], configKeyCRight ),
    DEF_OPT_BIND( bindStr[12], configKeyStickUp ),
    DEF_OPT_BIND( bindStr[13], configKeyStickDown ),
    DEF_OPT_BIND( bindStr[14], configKeyStickLeft ),
    DEF_OPT_BIND( bindStr[15], configKeyStickRight ),
    // max deadzone is 31000; this is less than the max range of ~32768, but this
    // way, the player can't accidentally lock themselves out of using the stick
    DEF_OPT_SCROLL( bindStr[16], &configStickDeadzone, 0, 100, 1 ),
    DEF_OPT_SCROLL( bindStr[17], &configRumbleStrength, 0, 100, 1)
};

static struct Option optsVideo[] = {
    #ifndef TARGET_SWITCH
    DEF_OPT_TOGGLE( optsVideoStr[0], &configWindow.fullscreen ),
    DEF_OPT_TOGGLE( optsVideoStr[5], &configWindow.vsync ),
    #endif
    DEF_OPT_CHOICE( optsVideoStr[1], &configFiltering, filterChoices ),
    DEF_OPT_TOGGLE( optsVideoStr[7], &configHUD ),
    #ifndef TARGET_SWITCH
    DEF_OPT_BUTTON( optsVideoStr[4], optvideo_reset_window ),
    DEF_OPT_BUTTON( optsVideoStr[9], optvideo_apply ),
    #endif
};

static struct Option optsGame[] = {
    DEF_OPT_CHOICE( optsGameStr[0], &configLanguage, NULL ),
    DEF_OPT_TOGGLE( optsGameStr[1], &configPrecacheRes ),
    #ifdef TARGET_SWITCH
    DEF_OPT_TOGGLE( optsGameStr[2], &configSwitchHud ),
    #endif
};

static struct Option optsAudio[] = {
    DEF_OPT_SCROLL( optsAudioStr[0], &configMasterVolume, 0, MAX_VOLUME, 1 ),
    DEF_OPT_SCROLL( optsAudioStr[1], &configMusicVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_SCROLL( optsAudioStr[2], &configSfxVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_SCROLL( optsAudioStr[3], &configEnvVolume, 0, MAX_VOLUME, 1),
};

static struct Option optsCheats[] = {
    DEF_OPT_TOGGLE( optsCheatsStr[0], &Cheats.EnableCheats ),
    DEF_OPT_TOGGLE( optsCheatsStr[1], &Cheats.MoonJump ),
    DEF_OPT_TOGGLE( optsCheatsStr[2], &Cheats.GodMode ),
    DEF_OPT_TOGGLE( optsCheatsStr[3], &Cheats.InfiniteLives ),
    DEF_OPT_TOGGLE( optsCheatsStr[4], &Cheats.SuperSpeed ),
    DEF_OPT_TOGGLE( optsCheatsStr[5], &Cheats.Responsive ),
    DEF_OPT_TOGGLE( optsCheatsStr[6], &Cheats.ExitAnywhere ),
    DEF_OPT_TOGGLE( optsCheatsStr[7], &Cheats.HugeMario ),
    DEF_OPT_TOGGLE( optsCheatsStr[8], &Cheats.TinyMario ),

};

/* submenu definitions */

#ifdef BETTERCAMERA
static struct SubMenu menuCamera   = DEF_SUBMENU( menuStr[4], optsCamera );
#endif
static struct SubMenu menuGame     = DEF_SUBMENU( menuStr[10], optsGame );
static struct SubMenu menuControls = DEF_SUBMENU( menuStr[5], optsControls );
static struct SubMenu menuVideo    = DEF_SUBMENU( menuStr[6], optsVideo );
static struct SubMenu menuAudio    = DEF_SUBMENU( menuStr[7], optsAudio );
static struct SubMenu menuCheats   = DEF_SUBMENU( menuStr[9], optsCheats );

/* main options menu definition */

static struct Option optsMain[] = {
    DEF_OPT_SUBMENU( menuStr[10], &menuGame ),
#ifdef BETTERCAMERA
    DEF_OPT_SUBMENU( menuStr[4], &menuCamera ),
#endif
    DEF_OPT_SUBMENU( menuStr[5], &menuControls ),
    DEF_OPT_SUBMENU( menuStr[6], &menuVideo ),
    DEF_OPT_SUBMENU( menuStr[7], &menuAudio ),
    DEF_OPT_BUTTON ( menuStr[8], optmenu_act_exit ),
    // NOTE: always keep cheats the last option here because of the half-assed way I toggle them
    DEF_OPT_SUBMENU( menuStr[9], &menuCheats )
};

static struct SubMenu mainOptions[] = {
#ifdef BETTERCAMERA
    DEF_SUBMENU( menuStr[4], optsCamera ),
#endif
    // DEF_SUBMENU( menuStr[10], optsGame ),
    DEF_SUBMENU( menuStr[5], optsControls ),
    DEF_SUBMENU( menuStr[6], optsVideo ),
    DEF_SUBMENU( menuStr[7], optsAudio ),
    DEF_SUBMENU( menuStr[9], optsCheats )
};

/* implementation */

static s32 optmenu_option_timer = 0;
static u8 optmenu_hold_count = 0;
static u8 current_index = 0;

static struct SubMenu *currentMenu;

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
static void optmenu_draw_box(s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b, u8 a) {
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

static void optmenu_draw_scaled_text(f32 x, f32 y, const u8 *str, int col, float scale) {    
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    moon_draw_text(x + 1, y-1, str, scale);
    if (col == -1) {
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else if (col == 0) {
        gDPSetEnvColor(gDisplayListHead++, 255, 32, 32, 255);
    } else if (col == 1) {
        gDPSetEnvColor(gDisplayListHead++, 32, 255, 32, 255);
    } else {
        gDPSetEnvColor(gDisplayListHead++, 0, 255, 242, 255);        
    }
    moon_draw_text(x, y, str, scale);    
}

#include <stdarg.h>

u8* concat(u8* a, u8* b) {
    int a_size = sizeof(a) / sizeof(a[0]);
    int b_size = sizeof(b) / sizeof(b[0]);;

    u8* tmp = malloc((a_size + b_size + 1) * sizeof(u8));
    memcpy(tmp,          a, (a_size) * sizeof(u8));
    memcpy(tmp + b_size, b, b_size * sizeof(u8));
    tmp[a_size + b_size] = 0xFF;
    return tmp;
}

u8* join(u8* a, u8* b, u8* c) {     
    int a_size = sizeof(a) / sizeof(a[0]);
    int b_size = sizeof(b) / sizeof(b[0]);
    int c_size = sizeof(c) / sizeof(c[0]);

    u8* tmp = malloc((a_size + c_size + b_size + 1) * sizeof(u8));
    memcpy(tmp,                   a, a_size * sizeof(u8));
    memcpy(tmp + c_size,          c, c_size * sizeof(u8));
    memcpy(tmp + c_size + b_size, b, b_size * sizeof(u8));
    tmp[a_size + c_size + b_size] = 0xFF;
    return tmp;
}

static void optmenu_draw_opt(const struct Option *opt, s16 x, s16 y, u8 sel) {    
    u8 * choice;

    u8* base = (u8*)get_key_string(opt->label);        

    s16 sx = 0;
    s16 sy = 0;
    s16 sw = 0;
    s16 sh = 0;    
    u8* lbl;

    int width;
    u8* tmpText;
    u8 buf[32] = { 0 };
    float scale = 0.9f;

    if(opt->type == OPT_TOGGLE || opt->type == OPT_CHOICE) {
        lbl = base;
        tmpText = get_key_string( opt->type == OPT_TOGGLE ? toggleStr[(int)*opt->bval] : opt->choices[*opt->uval]);


        width = (moon_get_text_width(lbl, scale, FALSE) + 8 + moon_get_text_width(tmpText, scale, FALSE)) / 2;
        optmenu_draw_scaled_text(x - width, y, lbl, -1, scale);
        optmenu_draw_scaled_text(x - width + 1 + moon_get_text_width(lbl, scale, FALSE), y, getTranslatedText(":"), -1, scale);
        optmenu_draw_scaled_text(x - width + 9 + moon_get_text_width(lbl, scale, FALSE), y, tmpText, opt->type & OPT_TOGGLE ? (int)*opt->bval : 2, scale);
    }    

    switch (opt->type) {
        case OPT_BUTTON:
            lbl = base;
            width = moon_get_text_width(lbl, scale, FALSE) / 2;
            optmenu_draw_scaled_text(x - width, y, lbl, -1, scale);
            break;

        case OPT_SCROLL:
            sx = x - 127 / 2;
            sy = 209 - (y - 35);            
            sw = sx + (127.0 * (((*opt->uval * 1.0) + __FLT_MIN__) / (opt->scrMax * 1.0)));
            sh = sy + 7;

            lbl = base;
            int_to_str(*opt->uval, buf);

            width = (moon_get_text_width(lbl, scale, FALSE) + 9 + moon_get_text_width(buf, scale, FALSE) + 3) / 2;
            optmenu_draw_scaled_text(x - width, y, lbl, -1, scale);
            optmenu_draw_scaled_text(x - width + 1 + moon_get_text_width(lbl, scale, FALSE), y, getTranslatedText(":"), -1, scale);
            optmenu_draw_scaled_text(x - width + 9 + moon_get_text_width(lbl, scale, FALSE), y, buf, 2, scale);
            optmenu_draw_scaled_text(x - width + 9 + moon_get_text_width(lbl, scale, FALSE) + moon_get_text_width(buf, scale, FALSE) + 1, y, getTranslatedText("%"), 2, scale);
            break;

        case OPT_BIND:
            lbl = base;
            tmpText = getTranslatedText( "Test");

            width = (moon_get_text_width(lbl, scale, FALSE)) / 2;
            optmenu_draw_scaled_text(x - width, y, lbl, -1, scale);
            // optmenu_draw_scaled_text(x - width + 1 + moon_get_text_width(lbl, scale), y, getTranslatedText("-"), -1, scale);
            // optmenu_draw_scaled_text(x - width + 9 + moon_get_text_width(lbl, scale), y, tmpText, opt->type & OPT_TOGGLE ? (int)*opt->bval : 2, scale);
            
            int base_width = moon_get_text_width(lbl, scale, FALSE);
            int padding = 3;
            u8* txt;

            for (u8 i = 0; i < MAX_BINDS; ++i) {
                const u8 white = (sel && (optmenu_bind_idx == i));
                // TODO: button names
                if (opt->uval[i] == VK_INVALID) {
                    txt = get_key_string(bindStr[optmenu_binding && white ? 1 : 0]);
                    base_width += moon_get_text_width(txt, scale, FALSE) + padding;
                    optmenu_draw_scaled_text(x - width + base_width, y, txt, -1, scale);
                } else {                    
                    u8* txt = getTranslatedText("0000");
                    base_width += moon_get_text_width(txt, scale, FALSE) + padding;
                    optmenu_draw_scaled_text(x - width + base_width, y, txt, -1, scale);
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
            *opt->uval = wrap_add(*opt->uval, val, 0, strcmp(opt->label, optsGameStr[0]) == 0 ? languagesAmount - 1: opt->numChoices - 1);
            set_language(configLanguage);
            break;

        case OPT_SCROLL:
            *opt->uval = wrap_add(*opt->uval, opt->scrStep * val, opt->scrMin, opt->scrMax);
            break;

        case OPT_SUBMENU:
            opt->nextMenu->prev = currentMenu;
            currentMenu = opt->nextMenu;
            break;

        case OPT_BUTTON:
            if (opt->actionFn)
                opt->actionFn(opt, val);
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
    }
}

static inline s16 get_hudstr_centered_x(const s16 sx, const u8 *str) {
    const u8 *chr = str;
    s16 len = 0;
    while (*chr != GLOBAR_CHAR_TERMINATOR) ++chr, ++len;
    return sx - len * 6; // stride is 12
}

 void PDrintBox(s32 aX, s32 aY, s32 aWidth, s32 aHeight, u32 aColorRGBA, bool aAlignLeft) {
    if ((aColorRGBA && 0xFF) != 0) {
        Mtx *_Matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (!_Matrix) return;
        if (aAlignLeft) {
            create_dl_translation_matrix(MENU_MTX_PUSH, aX, aY + aHeight, 0);
        } else {
            create_dl_translation_matrix(MENU_MTX_PUSH, aX + aWidth, aY + aHeight, 0);
        }
        guScale(_Matrix, (f32) aWidth / 130.f, (f32) aHeight / 80.f, 1.f);
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(_Matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
        gDPSetEnvColor(gDisplayListHead++, ((aColorRGBA >> 24) & 0xFF), ((aColorRGBA >> 16) & 0xFF), ((aColorRGBA >> 8) & 0xFF), ((aColorRGBA >> 0) & 0xFF));
        gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
        gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    }
}

float gSwitchValue = 0;
float gSwitchNewValue = 0;
float gGlobal = 0;

#define _POSIX_C_SOURCE 199309L
#include <sys/time.h>

//Options menu
void optmenu_draw(void) {
    gGlobal += 1.0f;
    s16 scroll;
    s16 scrollpos;    

    float range = 0.5f;
    float step = 0.1f;

    if(gSwitchValue >= range) 
        gSwitchNewValue -= step;
    else if (gSwitchValue <= -range)
        gSwitchNewValue += step;
    
    gSwitchValue = lerp(gSwitchValue, gSwitchNewValue, 0.01f);

    u8* label = get_key_string(currentMenu->label);
    float txtW = moon_get_text_width(label, 1.0, TRUE);

    const s16 labelX = SCREEN_WIDTH / 2 - txtW / 2;
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 30, 30, 30, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, labelX, 22, label);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, labelX, 20, label);

    optmenu_draw_scaled_text(labelX - 50 - gSwitchValue, SCREEN_HEIGHT - 34, getTranslatedText("<"), -1, 0.6f);
    optmenu_draw_scaled_text(labelX + txtW + 50 + gSwitchValue, SCREEN_HEIGHT - 34, getTranslatedText(">"), -1, 0.6f);
    
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    PDrintBox(25, 50, SCREEN_WIDTH - 50, SCREEN_HEIGHT * 0.6, 0x00000080, true);

    int base = 175;
    int padding = 15;
    for (u8 i = 0; i < currentMenu->numOpts; i++) {
        scroll = base - padding * i + currentMenu->scroll * padding;
        // FIXME: just start from the first visible option bruh
        if (scroll <= base && scroll > padding) {
            if((currentMenu->select == i))
                PDrintBox(30, 174 - (15 * i), SCREEN_WIDTH - 60, padding, 0xFFF20040, true);
            
            gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

            optmenu_draw_opt(&currentMenu->opts[i], 160, scroll, (currentMenu->select == i));
            gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
            gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
        }
    }
}

//This has been separated for interesting reasons. Don't question it.
void optmenu_draw_prompt(void) {
    //gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    //optmenu_draw_text(264, 212, get_key_string(menuStr[1 + optmenu_open]), 0);
    //gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

void optmenu_toggle(void) {
    if (optmenu_open == 0) {
        #ifndef nosound
        play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
        #endif

        currentMenu = &mainOptions[current_index];

        // HACK: hide the last option in main if cheats are disabled
        currentMenu->numOpts = sizeof(optsMain) / sizeof(optsMain[0]);
        if (!Cheats.EnableCheats) {
            currentMenu->numOpts--;
            if (currentMenu->select >= currentMenu->numOpts) {
                currentMenu->select = 0; // don't bother
                currentMenu->scroll = 0;
            }
        }
        
        optmenu_open = 1;
        
        /* Resets l_counter to 0 every time the options menu is open */
        l_counter = 0;
    } else {
        #ifndef nosound
        play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gDefaultSoundArgs);
        #endif
        optmenu_open = 0;
        #ifdef BETTERCAMERA
        newcam_init_settings(); // load bettercam settings from config vars
        #endif
        controller_reconfigure(); // rebind using new config values
        configfile_save(configfile_name());
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
    
    if(gPlayer1Controller->buttonPressed & L_CBUTTONS){
        int s  = sizeof(mainOptions) / sizeof(mainOptions[0]);
        if(current_index > 0) current_index--;
        else current_index = s - 1;
        currentMenu = &mainOptions[current_index];
    }
    if(gPlayer1Controller->buttonPressed & R_CBUTTONS){        
        int s  = sizeof(mainOptions) / sizeof(mainOptions[0]);

        if(current_index < s - 1) current_index++;
        else current_index = 0;
        currentMenu = &mainOptions[current_index];
    }

    /* Enables cheats if the user press the L trigger 3 times while in the options menu. Also plays a sound. */
    
    if ((gPlayer1Controller->buttonPressed & L_TRIG) && !Cheats.EnableCheats) {
        if (l_counter == 2) {
                Cheats.EnableCheats = true;
                play_sound(SOUND_MENU_STAR_SOUND, gDefaultSoundArgs);
                l_counter = 0;
        } else {
            l_counter++;
        }
    }
    
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

            //TODO: Fix this thing - Moon64
            // if (currentMenu->select < currentMenu->scroll)
            //     currentMenu->scroll = currentMenu->select;
            // else if (currentMenu->select > currentMenu->scroll + 3)
            //     currentMenu->scroll = currentMenu->select - 3;
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
