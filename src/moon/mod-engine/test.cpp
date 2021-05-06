#include "test.h"
#include <string>
#include <iostream>

extern "C" {
#include "moon/libs/lua/lua.h"
#include "moon/libs/lua/lualib.h"
#include "moon/libs/lua/lauxlib.h"
}

#include "moon/mod-engine/interfaces/mod-module.h"
#include "moon/mod-engine/modules/test-module.h"

using namespace std;

bool CheckLua(lua_State *L, int r) {
	if (r != LUA_OK)
	{
		std::string errormsg = lua_tostring(L, -1);
		std::cout << errormsg << std::endl;
		return false;
	}
	return true;
}

void CallLuaVoidFunction(lua_State *L, string function){
    lua_getglobal(L, function.c_str());
    if(lua_isfunction(L, -1)){
        if(!CheckLua(L, lua_pcall(L, 0, 0, 0))){
            std::cout << "There is an error executing [" << function << "]" << endl;
        }
    } else {
        std::cout << "[" << function << "] is not a function" << endl;
    }
}
int lastIndex = 0;

vector<ModModule*> mods = {
    new MathTestModule()
};

int luaopen_foo( lua_State *L )
{
    ModModule *m = mods[lastIndex];
    m->functions.push_back({NULL, NULL});
    luaL_newlib(L, m->functions.data());
    return 1;
}

void MoonLuaTest(){
    cout << "Testing Lua Engine" << endl;

    lua_State *L = luaL_newstate();
	// Add standard libraries to Lua Virtual Machine
    for(int i = 0; i < mods.size(); i++){
        lastIndex = i;
        luaL_requiref(L, mods[lastIndex]->moduleName.c_str(), luaopen_foo, 0);
    }
	luaL_openlibs(L);

    string file = "./build/us_pc/test.lua";
    if(CheckLua(L, luaL_dofile(L, file.c_str()))) {
        CallLuaVoidFunction(L, "onBitLoad");
        CallLuaVoidFunction(L, "onBitInit");
        CallLuaVoidFunction(L, "onBitExit");
    }
}