#ifndef ModEngineTextureModule
#define ModEngineTextureModule

#include "moon/mod-engine/interfaces/file-entry.h"
#include "moon/mod-engine/interfaces/bit-module.h"
#include <string>
#include <vector>
#include <map>

extern "C" {
#include "pc/gfx/gfx_pc.h"
}

namespace Moon {
    void saveAddonTexture(BitModule *addon, std::string texturePath, TextureFileEntry* data);
    void precacheBaseTexture(char* data, long size, std::string texturePath);
    TextureData *getCachedTexture(std::string texturePath);
}

namespace MoonInternal {
    TextureFileEntry *getTextureData(const char *fullpath);
    void loadTexture(int tile, const char *fullpath, struct GfxRenderingAPI *gfx_rapi);
    void saveTexture(TextureData *data, std::string texturePath);
    void buildTextureCache(std::vector<int> order);
    void buildDefaultAddon();
}

#endif