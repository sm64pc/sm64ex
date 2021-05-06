#include "moon/mod-engine/interfaces/mod-module.h"
#include "moon/mod-engine/modules/test-module.h"
#include "moon/mod-engine/interfaces/bit-module.h"

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
#include <vector>

using namespace std;

vector<BitModule*> addons;

extern "C" {
#include "moon/libs/lua/lualib.h"
#include "moon/libs/lua/lauxlib.h"
#include "moon/libs/lua/lua.h"
#include "text/libs/io_utils.h"
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

            if(file.has_file(bit->main)){
                std::cout << file.read(bit->main) << std::endl;
            }

            if(file.has_file("assets/")){
                for(auto &name : file.namelist()){
                    string graphicsPath = "assets/graphics/";
                    string textsPath = "assets/texts/";

                    if(!name.rfind(graphicsPath, 0)){
                        vector<string> allowedTextures = {"png", "jpg", "jpeg"};
                        if(std::count(allowedTextures.begin(), allowedTextures.end(), string(get_filename_ext(name.c_str())))){
                            string texName = name.substr(graphicsPath.length(), name.length() - 1);
                            // std::cout << texName << std::endl;
                        }
                    }
                    if(!name.rfind(textsPath, 0)){
                        if(!string(get_filename_ext(name.c_str())).compare("json")){
                            // std::cout << name << std::endl;
                        }
                    }
                }
            }
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

void Moon_InitModEngine(){
    Moon_LoadAddon("/home/alex/disks/uwu/Projects/UnderVolt/example.bit");
}