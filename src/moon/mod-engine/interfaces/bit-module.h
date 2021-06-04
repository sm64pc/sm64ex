#ifndef Moon64BitModule
#define Moon64BitModule
#include "file-entry.h"
#include "shader-entry.h"
#include <map>
#include <vector>
#include <string>

class zip_file;

extern "C" {
#include "pc/gfx/gfx_pc.h"
}

struct BitModule{
    std::string name;
    std::string description;
    std::vector<std::string> authors;
    double version;
    std::string website;
    std::string icon;
    std::string main;
    std::string path;
    std::map<std::string, EntryFileData*> textures;
    std::map<std::string, EntryFileData*> sounds;
    std::map<std::string, Shader*> shaders;
    bool readOnly;
    bool enabled;
};


#endif