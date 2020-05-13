#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include <SDL2/SDL.h>

// Analog camera movement by Pathétique (github.com/vrmiguel), y0shin and Mors
// Contribute or communicate bugs at github.com/vrmiguel/sm64-analog-camera

#include <ultra64.h>

#include "controller_api.h"

#include "../configfile.h"

extern int16_t rightx;
extern int16_t righty;

#ifdef BETTERCAMERA
int mouse_x;
int mouse_y;

extern u8 newcam_mouse;
#endif

static bool init_ok;
static SDL_GameController *sdl_cntrl;

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

    init_ok = true;
}

static void controller_sdl_read(OSContPad *pad) {
    if (!init_ok) {
        return;
    }

#ifdef BETTERCAMERA
    if (newcam_mouse == 1)
        SDL_SetRelativeMouseMode(SDL_TRUE);
    else
        SDL_SetRelativeMouseMode(SDL_FALSE);
    
    const u32 mbuttons = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
    
    if (configCameraMouse)
    {   
        if (configMouseA && (mbuttons & SDL_BUTTON(configMouseA))) pad->button |= A_BUTTON;
        if (configMouseB && (mbuttons & SDL_BUTTON(configMouseB))) pad->button |= B_BUTTON;
        if (configMouseL && (mbuttons & SDL_BUTTON(configMouseL))) pad->button |= L_TRIG;
        if (configMouseR && (mbuttons & SDL_BUTTON(configMouseR))) pad->button |= R_TRIG;
        if (configMouseZ && (mbuttons & SDL_BUTTON(configMouseZ))) pad->button |= Z_TRIG;
    }
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

    if (SDL_GameControllerGetButton(sdl_cntrl, configJoyStart)) pad->button |= START_BUTTON;
    if (SDL_GameControllerGetButton(sdl_cntrl, configJoyZ))     pad->button |= Z_TRIG;
    if (SDL_GameControllerGetButton(sdl_cntrl, configJoyL))     pad->button |= L_TRIG;
    if (SDL_GameControllerGetButton(sdl_cntrl, configJoyR))     pad->button |= R_TRIG;
    if (SDL_GameControllerGetButton(sdl_cntrl, configJoyA))     pad->button |= A_BUTTON;
    if (SDL_GameControllerGetButton(sdl_cntrl, configJoyB))     pad->button |= B_BUTTON;

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
    if (magnitude_sq > (uint32_t)(DEADZONE * DEADZONE)) {
        pad->stick_x = leftx / 0x100;
        int stick_y = -lefty / 0x100;
        pad->stick_y = stick_y == 128 ? 127 : stick_y;
    }
}

struct ControllerAPI controller_sdl = {
    controller_sdl_init,
    controller_sdl_read
};
