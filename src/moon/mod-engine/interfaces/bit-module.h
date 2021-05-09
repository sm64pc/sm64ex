#ifndef Moon64BitModule
#define Moon64BitModule
#include "file-entry.h"

#include <map>
#include <vector>
#include <string>

class zip_file;

extern "C" {
#include "pc/gfx/gfx_pc.h"
}
class BitModule{
public:
    std::string name;
    std::string description;
    std::string author;
    double version;
    std::string website;
    std::string icon;
    std::string main;
    std::string path;
    std::map<std::string, TextureFileEntry*> textures;
    bool readOnly;
    bool enabled;
};

#endif