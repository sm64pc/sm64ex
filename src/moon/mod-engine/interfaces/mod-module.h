#ifndef ModEngineModule
#define ModEngineModule

#include <string>
#include <vector>

extern "C" {
#include "moon/libs/lua/lualib.h"
#include "moon/libs/lua/lauxlib.h"
#include "moon/libs/lua/lua.h"
}

class ModModule {
public:
    std::string moduleName;
    std::vector<luaL_Reg> functions;
    ModModule(std::string moduleName){
        this->moduleName = moduleName;
    }
};

#endif