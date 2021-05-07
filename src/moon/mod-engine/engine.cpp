#include "engine.h"
#include "moon/mod-engine/interfaces/mod-module.h"
#include "moon/mod-engine/modules/test-module.h"
#include "interfaces/file-entry.h"

#include "moon/wrapper.h"
#include "moon/libs/nlohmann/json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <map>

using namespace std;

vector<BitModule*> addons;
map<string, TextureData*> textureMap;

map<string, TextureFileEntry*> baseGameTextures;

extern "C" {
#include "moon/libs/lua/lualib.h"
#include "moon/libs/lua/lauxlib.h"
#include "moon/libs/lua/lua.h"
#include "text/libs/io_utils.h"
}

void Moon_LoadAddonTextures(BitModule* module);

void Moon_LoadDefaultAddon(){
    BitModule* bit = new BitModule();
    bit->name        = "Moon64";
    bit->description = "SM64 Original Textures";
    bit->author      = "Nintendo";
    bit->version     = 1.0f;
    bit->readOnly    = true;
    bit->textures    = baseGameTextures;
    addons.push_back(bit);
    Moon_LoadAddonTextures(bit);
}

void Moon_LoadAddon(string path){
    miniz_cpp::zip_file file(path);

    if(file.has_file("properties.json")){
        string tjson = file.read("properties.json");

        json j = json::parse(tjson);
        if(j.contains("bit") && j["bit"].contains("name")){
            BitModule* bit = new BitModule();
            bit->name        = j["bit"]["name"];
            bit->description = j["bit"]["description"];
            bit->author      = j["bit"]["author"];
            bit->version     = j["bit"]["version"];
            bit->website     = j["bit"]["website"];
            bit->icon        = j["bit"]["icon"];
            bit->main        = j["bit"]["main"];
            bit->readOnly    = false;

            if(file.has_file(bit->main)){
                std::cout << file.read(bit->main) << std::endl;
            }

            if(file.has_file(bit->icon)){
                vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(bit->icon.c_str())))){
                    TextureFileEntry *entry = new TextureFileEntry();
                    file.read_texture(bit->icon, &entry);
                    bit->textures.insert(pair<string, TextureFileEntry*>("mod-icons://"+bit->name, entry));
                }
            }

            if(file.has_file("assets/")){
                for(auto &name : file.namelist()){
                    string graphicsPath = "assets/graphics/";
                    string textsPath = "assets/texts/";

                    if(!name.rfind(graphicsPath, 0)){
                        vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                        if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(name.c_str())))){
                            string texName = name.substr(graphicsPath.length());
                            string rawname = texName.substr(0, texName.find_last_of("."));
                            TextureFileEntry *entry = new TextureFileEntry();
                            file.read_texture(name, &entry);
                            bit->textures.insert(pair<string, TextureFileEntry*>(rawname, entry));
                        }
                    }
                    if(!name.rfind(textsPath, 0)){
                        if(!string(get_filename_ext(name.c_str())).compare("json")){
                            // std::cout << name << std::endl;
                        }
                    }
                }
            }

            if(!bit->textures.empty())
                Moon_LoadAddonTextures(bit);

            addons.push_back(bit);
        }
    } else {
        std::cout << "Failed to load addon: [" << file.get_filename() << "]" << std::endl;
        std::cout << "Missing properties.json" << std::endl;
    }
}

void Moon_ScanAddonsDirectory( char *exePath, char *gamedir ){
    string l_path(exePath);

    string languages_dir = l_path.substr(0, l_path.find_last_of("/\\")) + "/addons/";
    printf("Loading Directory: %s\n", languages_dir.c_str());

    // Scan directory for JSON files
    DIR *dir = opendir(languages_dir.c_str());
    if (dir) {
        struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            string extension = string(get_filename_ext(de->d_name));
            if (extension.compare("bit") == 0) {
                string file = languages_dir + de->d_name;
                printf("Loading Addon: %s\n", file.c_str());
                Moon_LoadAddon(file);
            }
        }
        closedir(dir);
    }
}

void Moon_LoadAddonTextures(BitModule* module){
    if(module->textures.empty()) return;
    cout << "Loading from " << module->name << " " << to_string(module->textures.size()) << " textures " << endl;

    for (map<string, TextureFileEntry*>::iterator entry = module->textures.begin(); entry != module->textures.end(); ++entry) {

        map<string, TextureData*>::iterator texIt;
        texIt = textureMap.find(entry->first);

        if(texIt != textureMap.end()){
            // cout << "Reloading: " << entry->first << endl;
            textureMap.erase(texIt);
        }

        TextureFileEntry* texEntry = entry->second;
        if(texEntry->data != nullptr) {
            overload_memory_texture(texEntry->data, texEntry->size, entry->first.c_str());
        }
    }
}

void Moon_SaveTexture(TextureData* data, string tex){
    textureMap.insert(pair<string, TextureData*>(tex, data));
}

void Moon_LoadBaseTexture(char* data, long size, string texture){
    if(baseGameTextures.find(texture) == baseGameTextures.end()){
        baseGameTextures.insert(pair<string, TextureFileEntry*>(texture, new TextureFileEntry({.size = size, .data = data})));
    }
}

TextureData* Moon_GetTexture(string texture){
    return textureMap.find(texture) != textureMap.end() ? textureMap.find(texture)->second : nullptr;
}

void Moon_TextFlyLoad(int id){
    switch(id){
        case 0:
            Moon_LoadDefaultAddon();
            break;
        case 1:
            Moon_LoadDefaultAddon();
            Moon_LoadAddon("/home/alex/Downloads/packs/converted/mc.bit");

            Moon_LoadAddon("/home/alex/Downloads/packs/converted/beta-hud.bit");
            Moon_LoadAddon("/home/alex/Downloads/packs/converted/moon64-demo.bit");
            break;
        case 2:
            Moon_LoadDefaultAddon();
            Moon_LoadAddon("/home/alex/Downloads/packs/converted/owo.bit");

            Moon_LoadAddon("/home/alex/Downloads/packs/converted/beta-hud.bit");
            Moon_LoadAddon("/home/alex/Downloads/packs/converted/moon64-demo.bit");
            break;
        case 3:
            Moon_LoadDefaultAddon();
            Moon_LoadAddon("/home/alex/Downloads/packs/converted/r96-hd.bit");

            Moon_LoadAddon("/home/alex/Downloads/packs/converted/beta-hud.bit");
            Moon_LoadAddon("/home/alex/Downloads/packs/converted/moon64-demo.bit");
            break;
    }
}

void Moon_PreInitModEngine(){
    Moon_LoadDefaultAddon();
    Moon_LoadAddon("/home/alex/Downloads/packs/converted/mc.bit");
    Moon_LoadAddon("/home/alex/Downloads/packs/converted/owo.bit");
    Moon_LoadAddon("/home/alex/Downloads/packs/converted/r96-hd.bit");
    Moon_LoadAddon("/home/alex/Downloads/packs/converted/beta-hud.bit");
    Moon_LoadAddon("/home/alex/Downloads/packs/converted/moon64-demo.bit");
}

void Moon_InitModEngine(){

}