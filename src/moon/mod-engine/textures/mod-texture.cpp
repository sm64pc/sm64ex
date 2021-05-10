#include "mod-texture.h"

#include "moon/zip/straw.h"
#include "moon/libs/nlohmann/json.hpp"
#include "moon/mod-engine/engine.h"
#include "assets/missing.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

extern "C" {
#include "text/libs/io_utils.h"
#include "pc/gfx/gfx_rendering_api.h"
#include "pc/platform.h"
#include "pc/fs/fs.h"
}

#include <iostream>
#include <vector>
#include <map>

using namespace std;
using json = nlohmann::json;

std::map<std::string, TextureFileEntry*> baseGameTextures;
map<string, TextureData*> textureMap;
map<string, BitModule*> textureCache;

namespace Moon {
    void saveAddonTexture(BitModule *addon, std::string texturePath, TextureFileEntry* data){
        addon->textures.insert(pair<string, TextureFileEntry*>(texturePath, data));
    }

    void precacheBaseTexture(char* data, long size, std::string texturePath){
        if(baseGameTextures.find(texturePath) == baseGameTextures.end())
            baseGameTextures.insert(pair<string, TextureFileEntry*>(
                texturePath,
                new TextureFileEntry({.path = "", .size = size, .data = data}))
            );
    }
    TextureData *getCachedTexture(string texturePath){
        return textureMap.find(texturePath) != textureMap.end() ? textureMap.find(texturePath)->second : nullptr;
    }
}

namespace MoonInternal {

    TextureFileEntry *getTextureData(const char *fullpath){
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
                    StrawFile file(addon->path);
                    TextureFileEntry *newData = new TextureFileEntry();
                    file.read(fileEntry->path, newData);
                    data = newData;
                }
            }
        }
        return data;
    }

    void loadTexture(int tile, const char *fullpath, struct GfxRenderingAPI *gfx_rapi){

        int w, h;
        u64 imgsize = 0;

        TextureFileEntry * imgdata = getTextureData(fullpath);
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

    void saveTexture(TextureData *data, string texturePath){
        textureMap.insert(pair<string, TextureData*>(texturePath, data));
    }

    void buildTextureCache(vector<int> order){
        textureCache.clear();

        for(int i=0; i < order.size(); i++){
            BitModule *addon = Moon::addons[order[i]];

            for (map<string, TextureFileEntry*>::iterator entry = addon->textures.begin(); entry != addon->textures.end(); ++entry) {
                auto texIt = textureCache.find(entry->first);
                if(texIt != textureCache.end()) textureCache.erase(texIt);

                textureCache.insert(pair<string, BitModule*>(entry->first, addon));
            }
        }
        textureMap.clear();
    }

    void buildDefaultAddon(){
        BitModule* bit = new BitModule();
        bit->name        = "Moon64";
        bit->description = "SM64 Original Textures";
        bit->author      = "Nintendo";
        bit->version     = 1.0f;
        bit->readOnly    = true;
        bit->textures    = baseGameTextures;
        Moon::addons.push_back(bit);
    }

}