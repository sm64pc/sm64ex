#ifndef Moon64BitModule
#define Moon64BitModule
#include "file-entry.h"

#include <map>
#include <vector>
#include <string>

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
    std::map<std::string, TextureFileEntry*> textures;
    // GFXTextureCache* textureCache;
    bool readOnly;
    bool enabled;
};

#endif