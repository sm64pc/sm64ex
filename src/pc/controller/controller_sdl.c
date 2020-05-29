#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include <SDL2/SDL.h>

// Analog camera movement by Pathétique (github.com/vrmiguel), y0shin and Mors
// Contribute or communicate bugs at github.com/vrmiguel/sm64-analog-camera

#include <ultra64.h>

#include "controller_api.h"
#include "controller_sdl.h"
#include "../configfile.h"

#include "game/level_update.h"

// mouse buttons are also in the controller namespace (why), just offset 0x100
#define VK_OFS_SDL_MOUSE 0x0100
#define VK_BASE_SDL_MOUSE (VK_BASE_SDL_GAMEPAD + VK_OFS_SDL_MOUSE)
#define MAX_JOYBINDS 32
#define MAX_MOUSEBUTTONS 8 // arbitrary

extern int16_t rightx;
extern int16_t righty;

#ifdef BETTERCAMERA
int mouse_x;
int mouse_y;

extern u8 newcam_mouse;
#endif

static bool init_ok;
static SDL_GameController *sdl_cntrl;


static u32 num_joy_binds = 0;
static u32 num_mouse_binds = 0;
static u32 joy_binds[MAX_JOYBINDS][2];
static u32 mouse_binds[MAX_JOYBINDS][2];

static bool joy_buttons[SDL_CONTROLLER_BUTTON_MAX ] = { false };
static u32 mouse_buttons = 0;
static u32 last_mouse = VK_INVALID;
static u32 last_joybutton = VK_INVALID;

static inline void controller_add_binds(const u32 mask, const u32 *btns) {
    for (u32 i = 0; i < MAX_BINDS; ++i) {
        if (btns[i] >= VK_BASE_SDL_GAMEPAD && btns[i] <= VK_BASE_SDL_GAMEPAD + VK_SIZE) {
            if (btns[i] >= VK_BASE_SDL_MOUSE && num_joy_binds < MAX_JOYBINDS) {
                mouse_binds[num_mouse_binds][0] = btns[i] - VK_BASE_SDL_MOUSE;
                mouse_binds[num_mouse_binds][1] = mask;
                ++num_mouse_binds;
            } else if (num_mouse_binds < MAX_JOYBINDS) {
                joy_binds[num_joy_binds][0] = btns[i] - VK_BASE_SDL_GAMEPAD;
                joy_binds[num_joy_binds][1] = mask;
                ++num_joy_binds;
            }
        }
    }
}

static void controller_sdl_bind(void) {
    bzero(joy_binds, sizeof(joy_binds));
    bzero(mouse_binds, sizeof(mouse_binds));
    num_joy_binds = 0;
    num_mouse_binds = 0;

    controller_add_binds(A_BUTTON,     configKeyA);
    controller_add_binds(B_BUTTON,     configKeyB);
    controller_add_binds(Z_TRIG,       configKeyZ);
    controller_add_binds(U_CBUTTONS,   configKeyCUp);
    controller_add_binds(L_CBUTTONS,   configKeyCLeft);
    controller_add_binds(D_CBUTTONS,   configKeyCDown);
    controller_add_binds(R_CBUTTONS,   configKeyCRight);
    controller_add_binds(L_TRIG,       configKeyL);
    controller_add_binds(R_TRIG,       configKeyR);
    controller_add_binds(START_BUTTON, configKeyStart);
}

static void controller_sdl_init(void) {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        return;
    }

#ifdef BETTERCAMERA
    if (newcam_mouse == 1)
        SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
#endif

    controller_sdl_bind();

    init_ok = true;
}

static void controller_sdl_read(OSContPad *pad) {
    if (!init_ok) {
        return;
    }

#ifdef BETTERCAMERA
    if (newcam_mouse == 1 && sCurrPlayMode != 2)
        SDL_SetRelativeMouseMode(SDL_TRUE);
    else
        SDL_SetRelativeMouseMode(SDL_FALSE);
    
    u32 mouse = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

    for (u32 i = 0; i < num_mouse_binds; ++i)
        if (mouse & SDL_BUTTON(mouse_binds[i][0]))
            pad->button |= mouse_binds[i][1];

    // remember buttons that changed from 0 to 1
    last_mouse = (mouse_buttons ^ mouse) & mouse;
    mouse_buttons = mouse;
    
#endif

    SDL_GameControllerUpdate();

    if (sdl_cntrl != NULL && !SDL_GameControllerGetAttached(sdl_cntrl)) {
        SDL_GameControllerClose(sdl_cntrl);
        sdl_cntrl = NULL;
    }
    if (sdl_cntrl == NULL) {
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGameController(i)) {
                sdl_cntrl = SDL_GameControllerOpen(i);
                if (sdl_cntrl != NULL) {
                    break;
                }
            }
        }
        if (sdl_cntrl == NULL) {
            return;
        }
    }

    for (u32 i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
        const bool new = SDL_GameControllerGetButton(sdl_cntrl, i);
        const bool pressed = !joy_buttons[i] && new;
        joy_buttons[i] = new;
        if (pressed) last_joybutton = i;
    }

    for (u32 i = 0; i < num_joy_binds; ++i)
        if (joy_buttons[joy_binds[i][0]])
            pad->button |= joy_binds[i][1];

    int16_t leftx = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_LEFTX);
    int16_t lefty = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_LEFTY);
    rightx = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_RIGHTX);
    righty = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_RIGHTY);

    int16_t ltrig = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    int16_t rtrig = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

#ifdef TARGET_WEB
    // Firefox has a bug: https://bugzilla.mozilla.org/show_bug.cgi?id=1606562
    // It sets down y to 32768.0f / 32767.0f, which is greater than the allowed 1.0f,
    // which SDL then converts to a int16_t by multiplying by 32767.0f, which overflows into -32768.
    // Maximum up will hence never become -32768 with the current version of SDL2,
    // so this workaround should be safe in compliant browsers.
    if (lefty == -32768) {
        lefty = 32767;
    }
    if (righty == -32768) {
        righty = 32767;
    }
#endif

    if (rightx < -0x4000) pad->button |= L_CBUTTONS;
    if (rightx > 0x4000) pad->button |= R_CBUTTONS;
    if (righty < -0x4000) pad->button |= U_CBUTTONS;
    if (righty > 0x4000) pad->button |= D_CBUTTONS;

    if (ltrig > 30 * 256) pad->button |= Z_TRIG;
    if (rtrig > 30 * 256) pad->button |= R_TRIG;

    uint32_t magnitude_sq = (uint32_t)(leftx * leftx) + (uint32_t)(lefty * lefty);
    uint32_t stickDeadzoneActual = configStickDeadzone * DEADZONE_STEP;
    if (magnitude_sq > (uint32_t)(stickDeadzoneActual * stickDeadzoneActual)) {
        pad->stick_x = leftx / 0x100;
        int stick_y = -lefty / 0x100;
        pad->stick_y = stick_y == 128 ? 127 : stick_y;
    }
}

static u32 controller_sdl_rawkey(void) {
    if (last_joybutton != VK_INVALID) {
        const u32 ret = last_joybutton;
        last_joybutton = VK_INVALID;
        return ret;
    }

    for (int i = 0; i < MAX_MOUSEBUTTONS; ++i) {
        if (last_mouse & SDL_BUTTON(i)) {
            const u32 ret = VK_OFS_SDL_MOUSE + i;
            last_mouse = 0;
            return ret;
        }
    }
    return VK_INVALID;
}

static void controller_sdl_shutdown(void) {
    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER)) {
        if (sdl_cntrl) {
            SDL_GameControllerClose(sdl_cntrl);
            sdl_cntrl = NULL;
        }
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }
    init_ok = false;
}

struct ControllerAPI controller_sdl = {
    VK_BASE_SDL_GAMEPAD,
    controller_sdl_init,
    controller_sdl_read,
    controller_sdl_rawkey,
    controller_sdl_bind,
    controller_sdl_shutdown
};
