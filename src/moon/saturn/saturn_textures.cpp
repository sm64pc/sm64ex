#include "saturn_textures.h"
#include "saturn.h"
#include "moon/mod-engine/hooks/hook.h"

#include "moon/utils/moon-env.h"
#include "moon/fs/moonfs.h"

#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include <dirent.h>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

extern "C" {
#include "moon/utils/moon-gfx.h"
#include "game/mario.h"
}

string custom_eye_name;
string custom_sky_name;

std::vector<string> eye_array;
string eyeDir;

// Textures

void saturn_eye_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_eyes_left_unused.rgba16")
            (*hookTexture) = const_cast<char*>(custom_eye_name.c_str());
    }});
}

void saturn_load_eye_array() {
    eye_array.clear();

    string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
#ifdef __MINGW32__
    // Windows moment
    eyeDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\graphics\\eyes\\";
#else
    eyeDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/graphics/eyes/";
#endif

    for (const auto & entry : fs::directory_iterator(eyeDir)) {
        size_t last_index = entry.path().filename().u8string().find_last_of("."); 
        string stripped_name = entry.path().filename().u8string().substr(0, last_index); 
        eye_array.push_back(stripped_name);
    }
}

void saturn_toggle_m_cap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_logo.rgba16") {
            if (enable_cap_logo) {
                (*hookTexture) = const_cast<char*>("actors/mario/mario_logo.rgba16");
            } else {
                (*hookTexture) = const_cast<char*>("blank");
            }
        }
    }});
}

void saturn_toggle_m_buttons() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_overalls_button.rgba16") {
            if (enable_overall_buttons) {
                (*hookTexture) = const_cast<char*>("actors/mario/mario_overalls_button.rgba16");
            } else {
                (*hookTexture) = const_cast<char*>("blank");
            }
        }
    }});
}

void saturn_sky_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName.find("textures/skyboxes/") != string::npos) {
            if (custom_sky_name != "default") {
                (*hookTexture) = const_cast<char*>(custom_sky_name.c_str());
            } else {
                (*hookTexture) = const_cast<char*>(texName.c_str());
            }
        }
    }});
}

/*
void saturn_toggle_night_skybox() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName.find("textures/skyboxes/") != string::npos) {
            if (enable_night_skybox) {
                (*hookTexture) = const_cast<char*>("night_skybox");
            } else {
                (*hookTexture) = const_cast<char*>(texName.c_str());
            }
        }
    }});
}
*/