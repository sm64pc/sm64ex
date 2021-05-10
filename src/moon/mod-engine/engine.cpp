#include "engine.h"

#include "moon/utils/moon-env.h"
#include "interfaces/file-entry.h"
#include "textures/mod-texture.h"

#include "moon/zip/straw.h"
#include "moon/libs/nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;

#include <iostream>
#include <string>
#include <limits.h>
#include <dirent.h>

extern "C" {
#include "text/libs/io_utils.h"
#include "pc/platform.h"
#include "pc/fs/fs.h"
}

namespace Moon {

    vector<BitModule*> addons;

    void loadAddon(string addonPath){
        StrawFile file(addonPath);

        cout << "Loading addon: " << addonPath << endl;

        if(file.exists("properties.json")){
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
                bit->path        = addonPath;
                bit->readOnly    = false;

                if(file.exists(bit->main)){
                    std::cout << file.read(bit->main) << std::endl;
                }

                if(file.exists(bit->icon)){
                    vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                    if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(bit->icon.c_str())))){
                        Moon::saveAddonTexture(bit, "mod-icons://"+bit->name, new TextureFileEntry({.path = bit->icon}));
                    }
                }

                if(file.exists("assets/")){
                    for(auto &name : file.entries()){
                        string graphicsPath = "assets/graphics/";
                        string textsPath = "assets/texts/";

                        if(!name.rfind(graphicsPath, 0)){
                            vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                            if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(name.c_str())))){
                                string texName = name.substr(graphicsPath.length());
                                string rawname = texName.substr(0, texName.find_last_of("."));

                                TextureFileEntry *entry = new TextureFileEntry();
                                file.read(name, entry);
                                Moon::saveAddonTexture(bit, rawname, entry);
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
            std::cout << "Failed to load addon: [" << file.getPath() << "]" << std::endl;
            std::cout << "Missing properties.json" << std::endl;
        }
    }
}

namespace MoonInternal {
     void scanAddonsDirectory() {
        string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
        string addonsDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/";

        DIR *dir = opendir(addonsDir.c_str());
        if (dir) {
            struct dirent *de;
            while ((de = readdir(dir)) != NULL) {
                string extension = string(get_filename_ext(de->d_name));
                if (extension.compare("bit") == 0) {
                    string file = addonsDir + de->d_name;
                    Moon::loadAddon(file);
                }
            }
            closedir(dir);
        }
    }

    void setupModEngine( string state ){
        if(state == "PreInit"){
            MoonInternal::buildDefaultAddon();
            return;
        }
        if(state == "Init"){
            MoonInternal::scanAddonsDirectory();
            vector<int> order;
            for(int i = 0; i < Moon::addons.size(); i++) order.push_back(i);
            MoonInternal::buildTextureCache(order);
            return;
        }
    }
}