#include "engine.h"

#include "moon/utils/moon-env.h"
#include "interfaces/file-entry.h"
#include "textures/mod-texture.h"
#include "shaders/mod-shaders.h"

#include "moon/fs/moonfs.h"
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
#include "pc/configfile.h"
}

namespace Moon {

    vector<BitModule*> addons;

    void loadAddon(string addonPath){
        MoonFS file(addonPath);
        file.open();

        cout << "Loading addon: " << addonPath << endl;

        if(file.exists("properties.json")){
            string tjson = file.read("properties.json");

            json j = json::parse(tjson);
            if(j.contains("bit") && j["bit"].contains("name")){
                BitModule* bit = new BitModule({
                    .name        = j["bit"]["name"],
                    .description = j["bit"].contains("icon") ? j["bit"]["description"] : "None",
                    .authors     = j["bit"]["authors"],
                    .version     = j["bit"]["version"],
                    .website     = j["bit"].contains("website") ? j["bit"]["website"]  : "None",
                    .icon        = j["bit"].contains("icon")    ? j["bit"]["icon"]     : "None"
                });

                if(j["bit"].contains("main")) bit->main = j["bit"]["main"];

                bit->path        = addonPath;
                bit->readOnly    = false;

                if(file.exists(bit->icon)){
                    vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                    if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(bit->icon.c_str())))){
                        TextureFileEntry *entry = new TextureFileEntry();
                        file.read(bit->icon, entry);
                        Moon::saveAddonTexture(bit, "mod-icons://"+bit->name, entry);
                    }
                    if(!string(get_filename_ext(bit->icon.c_str())).compare("json")){
                        string modName = bit->icon.substr(bit->icon.length());
                        cout << "Found animated icon texture " << modName << endl;
                        json mods = json::parse(file.read(bit->icon));
                        for (json::iterator entry = mods.begin(); entry != mods.end(); ++entry) {
                            Moon::bindTextureModifier("mod-icons://"+bit->name, entry.key(), entry.value());
                        }
                    }
                }

                if(file.exists("assets/")){
                    for(auto &name : file.entries()){
                        string texturePaths[] = {
                            "assets/graphics/",
                            "assets/models/"
                        };

                        string shadersPath = "assets/shaders/";
                        string textsPath = "assets/texts/";
                        string fileExtension = string(get_filename_ext(name.c_str()));

                        for(auto &path : texturePaths){
                            if(!name.rfind(path, 0)){
                                vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                                if(std::count(allowedTextures.begin(), allowedTextures.end(), fileExtension)){
                                    string texName = name.substr(path.length());
                                    string rawname = texName.substr(0, texName.find_last_of("."));
                                    TextureFileEntry *entry;
                                    if(configPrecacheRes)
                                        file.read(name, entry = new TextureFileEntry());
                                    else
                                        entry = new TextureFileEntry({.path = name});
                                    Moon::saveAddonTexture(bit, rawname, entry);
                                }
                                if(!fileExtension.compare("json")){
                                    string modName = name.substr(path.length());
                                    cout << "Found animated texture " << modName << endl;
                                    json mods = json::parse(file.read(name));
                                    for (json::iterator entry = mods.begin(); entry != mods.end(); ++entry) {
                                    cout << "Binding " << entry.key() << endl;
                                        Moon::bindTextureModifier(modName.substr(0, modName.find_last_of(".")), entry.key(), entry.value());
                                    }
                                }
                            }
                        }
                        if(!name.rfind(shadersPath, 0)){
                            vector<string> shaderExts = { "vs", "fs" };
                            if(std::count(shaderExts.begin(), shaderExts.end(), fileExtension)){
                                string path = name.substr(shadersPath.length());
                                string raw = path.substr(0, path.find_last_of("."));
                                cout << "Found shader source: " << raw << endl;
                                // Moon::saveAddonShader(bit, raw, file.read(name), !fileExtension.compare("vs") ? ShaderType::VERTEX : ShaderType::FRAGMENT);
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
            std::cout << "addon: [" << file.getPath() << "]" << std::endl;
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
                if (de->d_name[0] != '.') {
                    string file = addonsDir + de->d_name;
                    Moon::loadAddon(file);
                }
            }
            closedir(dir);
        }
    }

    void setupModEngine( string state ){
        MoonInternal::setupTextureEngine(state);

        if(state == "PreStartup"){
            MoonInternal::scanAddonsDirectory();
            return;
        }
        if(state == "PreInit"){
            vector<int> order;
            for(int i = 0; i < Moon::addons.size(); i++) order.push_back(i);
            MoonInternal::buildTextureCache(order);
            return;
        }
        if(state == "Exit"){
            Moon::addons.clear();
            return;
        }
    }
}