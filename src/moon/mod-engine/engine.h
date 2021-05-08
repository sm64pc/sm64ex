#ifndef Moon64ModEngine
#define Moon64ModEngine

#include "moon/mod-engine/interfaces/bit-module.h"
#include <vector>

extern "C" {
#include "pc/gfx/gfx_pc.h"
}

extern std::vector<BitModule*> addons;

void Moon_SaveTexture(TextureData* data, std::string tex);
TextureData* Moon_GetTexture(std::string texture);
void Moon_PreInitModEngine();
void Moon_InitModEngine( char *exePath, char *gamedir );

void Moon_LoadBaseTexture(char* data, long size, std::string texture);


// TESTS
void Moon_TextFlyLoad(int id);
void Moon_TestRebuildOrder(std::vector<int> order);

#endif