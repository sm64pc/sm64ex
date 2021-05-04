#ifndef MoonIOManager
#define MoonIOManager

#include <iostream>
#include <vector>
#include "io-module.h"

using namespace std;
extern vector<MIOModule*> modules;

void InitIOModules();
void UpdateIOModules();
template< typename T > T* GetIOModule() {
    for(int id = 0; id < modules.size(); id++){
        T* m = dynamic_cast<T*>(modules[id]);
        if (m) return m;
    }

    return NULL;
}

#endif