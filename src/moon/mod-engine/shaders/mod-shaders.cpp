#include "mod-shaders.h"
#include "moon/mod-engine/interfaces/bit-module.h"
#include "moon/mod-engine/engine.h"
#include <iostream>
#include <string>
#include <map>

extern "C" {
#include "pc/platform.h"
}

using namespace std;

namespace Moon {
    void saveAddonShader(BitModule *addon, string shaderID, string shaderData, ShaderType type){
        if(addon->shaders.find(shaderID) == addon->shaders.end()) addon->shaders[shaderID] = new Shader();
        cout << "===========================================" << endl;
        cout << "ID: " << shaderID << endl;
        cout << "Type: " << ( type == FRAGMENT ? "Fragment" : "Vertex") << endl;
        cout << "===========================================" << endl;
        switch(type){
            case VERTEX:
                addon->shaders[shaderID]->vertexData = shaderData;
                break;
            case FRAGMENT:
                addon->shaders[shaderID]->fragmentData = shaderData;
                break;
        }
    }
}

extern "C" {
void compileShaders(unsigned int shaderProgram, void (*glCallback)(unsigned int shaderProgram, char** sources, unsigned long* length)){
    cout << Moon::addons.size() << endl;
    for(auto &addon : Moon::addons){
        for (map<string, Shader*>::iterator entry = addon->shaders.begin(); entry != addon->shaders.end(); ++entry) {
            cout << entry->second->vertexData.empty() << " - " << entry->second->fragmentData.empty() << endl;
            if(!entry->second->vertexData.empty() && !entry->second->fragmentData.empty()){
                vector<char*> sources = {
                    sys_strdup(entry->second->vertexData.c_str()),
                    sys_strdup(entry->second->fragmentData.c_str())
                };
                vector<unsigned long> lengths = {
                    entry->second->vertexData.size(),
                    entry->second->fragmentData.size()
                };
                printf("TEST %s\n", sources.data()[0]);
                (*glCallback)(shaderProgram, sources.data(), lengths.data());
            }
        }
    }
}
}