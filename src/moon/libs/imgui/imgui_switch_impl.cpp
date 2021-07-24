#include "imgui_switch_impl.h"
#include "moon/mod-engine/hooks/hook.h"

#include <SDL2/SDL.h>
#include <string>
#include <iostream>

#ifdef TARGET_SWITCH
#include <switch.h>
#endif

using namespace std;

namespace Platform {
#ifdef TARGET_SWITCH
    bool isSwitch = true;
#else
    bool isSwitch = false;
#endif
}

namespace MoonInternal {

    int mouseX = -9999;
    int mouseY = -9999;
    u32 mstate;

#ifdef TARGET_SWITCH
    HidTouchScreenState state;
    s32 prev_touchcount;
#endif

    void setupWindowHook(string status){
        if(status == "PreStartup"){
        #ifdef TARGET_SWITCH
            hidInitializeTouchScreen();
        #endif
            Moon::registerHookListener({.hookName = WINDOW_API_START_FRAME, .callback = [](HookCall call){
            #ifndef TARGET_SWITCH
                mstate = SDL_GetMouseState(&mouseX, &mouseY);
                cout << "MouseX: " << mouseX << " MouseY: " << mouseY << " Mouse State: " << mstate << endl;
            #else
                if (hidGetTouchScreenStates(&state, 1)) {
                    if (state.count != prev_touchcount) {
                        prev_touchcount = state.count;
                    }

                    if (state.count < prev_touchcount){
                        mstate = 0;
                    }

                    for(s32 i = 0; i < state.count; i++) {
                        mouseX = state.touches[0].x;
                        mouseY = state.touches[0].y;
                        mstate = 1;
                    }
                }
            #endif
        }
    }

    u32 Moon_GetMouseState(int *x, int *y){
        *x = mouseX;
        *y = mouseY;
        return mstate;
    }
}