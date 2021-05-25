#ifndef ModEngineHookModule
#define ModEngineHookModule

struct HookParameter {
    char* name;
    void* parameter;
};

#define TEXTURE_BIND  "TextureBind"
#define PRE_HUD_DRAW  "PreHudDraw"
#define HUD_DRAW      "HudDraw"
#define POST_HUD_DRAW "PostHudDraw"

#ifdef __cplusplus

#include <string>
#include <map>

struct HookCall {
    std::string name;
    std::map<std::string, void*> baseArgs;
    std::map<std::string, void*> hookedArgs;
    bool cancelled = false;
};

typedef void HookFunc(HookCall call);
struct HookListener {
    std::string hookName;
    HookFunc *callback;
    int priority = 0;
};

namespace Moon {
    void registerHookListener(HookListener listener);
}

#else

void moon_bind_hook(char* name);
void moon_init_hook(int length, ...);
bool moon_call_hook(int length, ...);

#endif
#endif