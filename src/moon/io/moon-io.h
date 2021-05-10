#ifndef MoonIOManager
#define MoonIOManager

#include "io-module.h"

#include <iostream>
#include <vector>
#include <string>

namespace Moon {
    extern std::vector<MIOModule*> modules;

    template< typename T > T* GetIOModule() {
        for(int id = 0; id < modules.size(); id++){
            T* m = dynamic_cast<T*>(modules[id]);
            if (m) return m;
        }

        return NULL;
    }

}

namespace MoonInternal {
    void setupIOModuleEngine( std::string state );
}
#endif