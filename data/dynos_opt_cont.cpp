#include "dynos.cpp.h"
extern "C" {
#include "pc/controller/controller_api.h"
}

static bool DynOS_Opt_ControllerIsKeyDown(s32 aCont, s32 aKey) {

    // Keyboard
    if (aCont == 0 && aKey >= 0 && aKey < SDL_NUM_SCANCODES) {
        return SDL_GetKeyboardState(NULL)[aKey];
    }

    // Game Controller
    else if (aKey >= 0x1000) {

        // Button
        s32 _Button = (aKey - 0x1000);
        if (_Button < SDL_CONTROLLER_BUTTON_MAX) {
            return SDL_GameControllerGetButton(SDL_GameControllerOpen(aCont - 1), SDL_GameControllerButton(_Button));
        }

        // Axis
        s32 _Axis = (aKey - 0x1000 - SDL_CONTROLLER_BUTTON_MAX);
        if (_Axis < SDL_CONTROLLER_AXIS_MAX * 2) {
            s32 _AxisValue = SDL_GameControllerGetAxis(SDL_GameControllerOpen(aCont - 1), SDL_GameControllerAxis(_Axis / 2));
            if (_Axis & 1) return (_AxisValue < (SHRT_MIN / 2));
            else           return (_AxisValue > (SHRT_MAX / 2));
        }
    }
    // Invalid
    return false;
}

#define MAX_CONTS 8
bool DynOS_Opt_ControllerUpdate(DynosOption *aOpt, void *aData) {
    if (aOpt->mType == DOPT_BIND) {
        OSContPad *pad = (OSContPad *) aData;
        for (s32 _Cont = 0; _Cont < MAX_CONTS; ++_Cont)
        for (s32 _Bind = 0; _Bind < 3; ++_Bind) {
            pad->button |= aOpt->mBind.mMask * DynOS_Opt_ControllerIsKeyDown(_Cont, aOpt->mBind.mBinds[_Bind]);
        }
    }
    return false;
}

#define MAX_GKEYS (SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_AXIS_MAX * 2)
s32 sBindingState = 0; // 0 = No bind, 1 = Wait for all keys released, 2 = Return first pressed key
s32 DynOS_Opt_ControllerGetKeyPressed() {

    // Keyboard
    for (s32 _Key = 0; _Key < SDL_NUM_SCANCODES; ++_Key) {
        if (DynOS_Opt_ControllerIsKeyDown(0, _Key)) {
            if (sBindingState == 1) return VK_INVALID;
            return _Key;
        }
    }

    // Game Controller
    for (s32 _Cont = 1; _Cont < MAX_CONTS; ++_Cont)
    for (s32 _Key = 0; _Key < MAX_GKEYS; ++_Key) {
        if (DynOS_Opt_ControllerIsKeyDown(_Cont, _Key + 0x1000)) {
            if (sBindingState == 1) return VK_INVALID;
            return _Key + 0x1000;
        }
    }

    // No key
    sBindingState = 2;
    return VK_INVALID;
}
