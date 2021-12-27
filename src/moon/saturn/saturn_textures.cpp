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
string custom_emblem_name;
string custom_stache_name;
string custom_button_name;
string custom_sideburn_name;
string custom_sky_name;

std::vector<string> eye_array;
string eyeDir;
std::vector<string> emblem_array;
string emblemDir;
std::vector<string> stache_array;
string stacheDir;
std::vector<string> button_array;
string buttonDir;
std::vector<string> sideburn_array;
string sideburnDir;

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
    eyeDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\graphics\\saturn\\eyes\\";
#else
    eyeDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/graphics/saturn/eyes/";
#endif

    for (const auto & entry : fs::directory_iterator(eyeDir)) {
        size_t last_index = entry.path().filename().u8string().find_last_of("."); 
        string stripped_name = entry.path().filename().u8string().substr(0, last_index); 
        eye_array.push_back(stripped_name);
    }
}

void saturn_emblem_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_logo.rgba16") {
            if (custom_emblem_name != "default") {
                (*hookTexture) = const_cast<char*>(custom_emblem_name.c_str());
            } else {
#ifdef __MINGW32__
                (*hookTexture) = const_cast<char*>("actors/mario/mario_logo.rgba16");
#else
                (*hookTexture) = const_cast<char*>(texName.c_str());
#endif
            }
        }
    }});
}
void saturn_load_emblem_array() {
    emblem_array.clear();

    string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
#ifdef __MINGW32__
    // Windows moment
    emblemDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\graphics\\saturn\\emblems\\";
#else
    emblemDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/graphics/saturn/emblems/";
#endif

    for (const auto & entry : fs::directory_iterator(emblemDir)) {
        size_t last_index = entry.path().filename().u8string().find_last_of("."); 
        string stripped_name = entry.path().filename().u8string().substr(0, last_index); 
        emblem_array.push_back(stripped_name);
    }
}

void saturn_stache_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_mustache.rgba16") {
            if (custom_stache_name != "default") {
                (*hookTexture) = const_cast<char*>(custom_stache_name.c_str());
            } else {
                (*hookTexture) = const_cast<char*>(texName.c_str());
            }
        }
    }});
}
void saturn_load_stache_array() {
    stache_array.clear();

    string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
#ifdef __MINGW32__
    // Windows moment
    stacheDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\graphics\\saturn\\staches\\";
#else
    stacheDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/graphics/saturn/staches/";
#endif

    for (const auto & entry : fs::directory_iterator(stacheDir)) {
        size_t last_index = entry.path().filename().u8string().find_last_of("."); 
        string stripped_name = entry.path().filename().u8string().substr(0, last_index); 
        stache_array.push_back(stripped_name);
    }
}

void saturn_button_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_overalls_button.rgba16") {
            if (custom_button_name != "default") {
                (*hookTexture) = const_cast<char*>(custom_button_name.c_str());
            } else {
                (*hookTexture) = const_cast<char*>(texName.c_str());
            }
        }
    }});
}
void saturn_load_button_array() {
    button_array.clear();

    string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
#ifdef __MINGW32__
    // Windows moment
    buttonDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\graphics\\saturn\\buttons\\";
#else
    buttonDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/graphics/saturn/buttons/";
#endif

    for (const auto & entry : fs::directory_iterator(buttonDir)) {
        size_t last_index = entry.path().filename().u8string().find_last_of("."); 
        string stripped_name = entry.path().filename().u8string().substr(0, last_index); 
        button_array.push_back(stripped_name);
    }
}

void saturn_sideburn_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "actors/mario/mario_sideburn.rgba16") {
            if (custom_sideburn_name != "default") {
                (*hookTexture) = const_cast<char*>(custom_sideburn_name.c_str());
            } else {
                (*hookTexture) = const_cast<char*>(texName.c_str());
            }
        }
    }});
}
void saturn_load_sideburn_array() {
    sideburn_array.clear();

    string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
#ifdef __MINGW32__
    // Windows moment
    sideburnDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\graphics\\saturn\\sideburns\\";
#else
    sideburnDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/graphics/saturn/sideburns/";
#endif

    for (const auto & entry : fs::directory_iterator(sideburnDir)) {
        size_t last_index = entry.path().filename().u8string().find_last_of("."); 
        string stripped_name = entry.path().filename().u8string().substr(0, last_index); 
        sideburn_array.push_back(stripped_name);
    }
}

void saturn_chroma_sky_swap() {
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call) {
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(texName == "textures/skyboxes/clouds.rgba16") {
            if (has_changed_chroma_sky) {
                (*hookTexture) = const_cast<char*>("saturn/white");
            } else {
                (*hookTexture) = const_cast<char*>(texName.c_str());
            }
        }
    }});
}