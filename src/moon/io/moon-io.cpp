#include "moon-io.h"
#include "moon/io/io-module.h"
#include "moon/io/modules/mouse-io.h"

vector<MIOModule*> modules;

void InitIOModules(){
    modules.push_back(new MouseIO());
    for(auto &module : modules) module->init();
}

void UpdateIOModules(){
    for(auto &module : modules) module->update();
}