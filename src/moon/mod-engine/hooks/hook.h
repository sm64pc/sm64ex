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

#define SAVE_GRAPH_NODE "SaveGraphNode"
#define LOAD_GRAPH_NODE "LoadGraphNode"

#define IMGUI_API_INIT "ImGuiApiInit"
#define IMGUI_API_DRAW "ImGuiApiDraw"

#define WINDOW_API_INIT  "WApiInit"
#define WINDOW_API_HANDLE_EVENTS  "WApiHandleEvents"
#define WINDOW_API_START_FRAME  "WApiStartFrame"

// Graphics API Hooks
#define GFX_PRE_START_FRAME  "GFXApiPreStartFrame"
#define GFX_POST_START_FRAME "GFXApiPostStartFrame"

#define GFX_PRE_END_FRAME  "GFXApiPreEndFrame"
#define GFX_POST_END_FRAME "GFXApiPostEndFrame"

#define GFX_ON_REZISE "GFXApiOnResize"
#define GFX_INIT      "GFXApiInit"
#define GFX_SHUTDOWN  "GFXApiShutdown"

// End

#ifdef __cplusplus

#include <functional>
#include <string>
#include <map>

struct HookCall {
    std::string name;
    std::map<std::string, void*> baseArgs;
    std::map<std::string, void*> hookedArgs;
    bool cancelled = false;
};

typedef std::function<void(HookCall)> HookFunc;
struct HookListener {
    std::string hookName;
    HookFunc callback;
    int priority = 0;
};

namespace Moon {
    void registerHookListener(HookListener listener);
}

namespace MoonInternal {
    void bindHook(std::string name);
    void initBindHook(int length, ...);
    bool callBindHook(int length, ...);
}

#else

void moon_bind_hook(char* name);
void moon_init_hook(int length, ...);
bool moon_call_hook(int length, ...);

#endif
#endif