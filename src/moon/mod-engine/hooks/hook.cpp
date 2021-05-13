#include "hook.h"
#include <map>
#include <string>
#include <vector>
#include <stdarg.h>
#include <iostream>

using namespace std;

map<string, vector<HookFunc*>> listeners;

namespace Moon {
    void registerHookListener(HookListener listener){
        listeners[listener.hookName].push_back(listener.callback);
    }
}

namespace MoonInternal {
    bool handleHook(HookCall call){
        bool cancelled = false;
        for (auto& listener : listeners[string(call.name)]){
            cancelled = (*listener)(call);
        }
        return cancelled;
    }
}

/*
#############################
    Module: Hook C Handle
#############################
*/

string hookName;
map<string, void*> initArgs;
map<string, void*> hookArgs;

extern "C" {

void moon_bind_hook(char* name){
    hookName = string(name);
}

void moon_init_hook(int length, ...){
    if(length > 0){
        va_list args;
        va_start(args, length);
        for(int i = 0; i < length; i++) {
            HookParameter currentParam = va_arg(args, struct HookParameter);
            initArgs[currentParam.name] = currentParam.parameter;
        }
        va_end(args);
    }
}

bool moon_call_hook(int length, ...){
    if(length > 0){
        va_list args;
        va_start(args, length);
        for(int i = 0; i < length; i++) {
            HookParameter currentParam = va_arg(args, struct HookParameter);
            hookArgs[currentParam.name] = currentParam.parameter;
        }
        va_end(args);
    }

    bool cancelled = MoonInternal::handleHook({
        .name = hookName,
        .baseArgs = initArgs,
        .hookedArgs = hookArgs
    });

    hookName = "";
    initArgs.clear();
    hookArgs.clear();

    return cancelled;
}

}