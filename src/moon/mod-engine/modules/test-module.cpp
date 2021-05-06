#include "test-module.h"

#include <iostream>

int testFunction(lua_State *L){
    std::cout << "Testing function" << std::endl;
}

MathTestModule::MathTestModule() : ModModule("test"){
    this->functions.push_back({"amul", testFunction});
    this->functions.push_back({"bmul", testFunction});
    this->functions.push_back({"cmul", testFunction});
    this->functions.push_back({"dmul", testFunction});
}