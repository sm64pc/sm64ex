#ifndef SWITCH_IMGUI_IMPL
#define SWITCH_IMGUI_IMPL

#include <string>

extern "C"{
#include "types.h"
}

namespace MoonInternal {
    void setupWindowHook(std::string status);
    u32 Moon_GetMouseState(int *x, int *y);
}

#endif