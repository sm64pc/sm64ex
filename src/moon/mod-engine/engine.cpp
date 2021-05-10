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
#include <chrono>
#include "missing.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace std;

vector<BitModule*> addons;
map<string, TextureData*> textureMap;

std::map<std::string, TextureFileEntry*> baseGameTextures;
map<string, BitModule*> textureCache;

extern "C" {
#include "moon/libs/lua/lualib.h"
#include "moon/libs/lua/lauxlib.h"
#include "moon/libs/lua/lua.h"
#include "text/libs/io_utils.h"
#include "pc/gfx/gfx_rendering_api.h"
#include "pc/platform.h"
#include "pc/fs/fs.h"
}

void Moon_LoadDefaultAddon(){
    BitModule* bit = new BitModule();
    bit->name        = "Moon64";
    bit->description = "SM64 Original Textures";
    bit->author      = "Nintendo";
    bit->version     = 1.0f;
    bit->readOnly    = true;
    bit->textures    = baseGameTextures;
    addons.push_back(bit);
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
            bit->path        = path;
            bit->readOnly    = false;

            if(file.has_file(bit->main)){
                std::cout << file.read(bit->main) << std::endl;
            }

            if(file.has_file(bit->icon)){
                vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(bit->icon.c_str())))){
                    bit->textures.insert(pair<string, TextureFileEntry*>("mod-icons://"+bit->name, new TextureFileEntry({.path = bit->icon})));
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

            addons.push_back(bit);
        }
    } else {
        std::cout << "Failed to load addon: [" << file.get_filename() << "]" << std::endl;
        std::cout << "Missing properties.json" << std::endl;
    }
}

void Moon_BakeTextureCache(vector<int> order){
    textureCache.clear();

    for(int i=0; i < order.size(); i++){
        BitModule *addon = addons[order[i]];

        for (map<string, TextureFileEntry*>::iterator entry = addon->textures.begin(); entry != addon->textures.end(); ++entry) {
            auto texIt = textureCache.find(entry->first);
            if(texIt != textureCache.end()) textureCache.erase(texIt);

            textureCache.insert(pair<string, BitModule*>(entry->first, addon));
        }
    }
    textureMap.clear();
}

TextureFileEntry *Moon_getTextureData(const char *fullpath){
    char texname[SYS_MAX_PATH];
    strncpy(texname, fullpath, sizeof(texname));
    texname[sizeof(texname)-1] = 0;
    char *dot = strrchr(texname, '.');
    if (dot) *dot = 0;

    char *actualname = texname;
    if (!strncmp(FS_TEXTUREDIR "/", actualname, 4)) actualname += 4;
    actualname = sys_strdup(actualname);
    assert(actualname);

    auto cacheEntry = textureCache.find(actualname);
    BitModule *addon = cacheEntry->second;

    TextureFileEntry * data = NULL;

    if(addon != NULL){
        TextureFileEntry *fileEntry = addon->textures.find(actualname)->second;

        if(fileEntry != NULL){
            if(fileEntry->data != NULL) data = fileEntry;
            else if(!fileEntry->path.empty()){
                miniz_cpp::zip_file file(addon->path);
                TextureFileEntry *newData = new TextureFileEntry();
                file.read_texture(fileEntry->path, &newData);
                data = newData;
            }
        }
    }
    return data;
}

void Moon_LoadTexture(int tile, const char *fullpath, struct GfxRenderingAPI *gfx_rapi){

    int w, h;
    u64 imgsize = 0;

    TextureFileEntry * imgdata = Moon_getTextureData(fullpath);
    if (imgdata) {
        u8 *data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(imgdata->data), imgdata->size, &w, &h, NULL, 4);
        if (data) {
            gfx_rapi->upload_texture(data, w, h);
            stbi_image_free(data);
            return;
        } else {
            cout << "Failed to convert texture" << endl;
            std::cout << stbi_failure_reason() << std::endl;
        }
    } else {
        cout << "Failed to load texture" << endl;
    }

    gfx_rapi->upload_texture(missing_image.pixel_data, missing_image.width, missing_image.height);
}

void Moon_ScanAddonsDirectory( char *exePath, char *gamedir ){
    string l_path(exePath);

    string languages_dir = l_path.substr(0, l_path.find_last_of("/\\")) + "/addons/";
    printf("Loading Directory: %s\n", languages_dir.c_str());

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

void Moon_SaveTexture(TextureData* data, string tex){
    textureMap.insert(pair<string, TextureData*>(tex, data));
}

void Moon_LoadBaseTexture(char* data, long size, string texture){
    if(baseGameTextures.find(texture) == baseGameTextures.end()){
        baseGameTextures.insert(pair<string, TextureFileEntry*>(texture, new TextureFileEntry({.path = "", .size = size, .data = data})));
    }
}

TextureData* Moon_GetTexture(string texture){
    return textureMap.find(texture) != textureMap.end() ? textureMap.find(texture)->second : nullptr;
}

void Moon_TextFlyLoad(int id){

}

void Moon_PreInitModEngine(){
    Moon_LoadDefaultAddon();
}

void Moon_TestRebuildOrder(vector<int> order){
    Moon_BakeTextureCache(order);
}

using namespace std::chrono;

void Moon_InitModEngine( char *exePath, char *gamedir ){

    milliseconds start_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
    Moon_ScanAddonsDirectory( exePath, gamedir );
    vector<int> order;
    for(int i = 0; i < addons.size(); i++) order.push_back(i);
    Moon_BakeTextureCache(order);
    milliseconds end_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );

    std::cout << "Finised loading in " << ((end_ms.count() - start_ms.count()) / 1000) << " seconds" << std::endl;
}