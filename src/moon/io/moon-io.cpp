#include "moon-io.h"
#include "moon/io/io-module.h"
#include "moon/io/modules/mouse-io.h"

using namespace std;

namespace Moon {
    vector<MIOModule*> modules;
}

namespace MoonInternal {
    void setupIOModuleEngine( string state ){
        if(state == "PreInit"){
            Moon::modules.push_back(new MouseIO());
            return;
        }
        if(state == "Init"){
            for(auto &module : Moon::modules) module->init();
            return;
        }
        if(state == "Update"){
            for(auto &module : Moon::modules) module->update();
            return;
        }
    }
}