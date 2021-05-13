#ifndef ModEngineHookModule
#define ModEngineHookModule

struct HookParameter {
    char* name;
    void* parameter;
};

#define TEXTURE_BIND "TextureBind"

#ifdef __cplusplus

#include <string>
#include <map>

struct HookCall {
    std::string name;
    std::map<std::string, void*> baseArgs;
    std::map<std::string, void*> hookedArgs;
};

typedef bool HookFunc(HookCall call);
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