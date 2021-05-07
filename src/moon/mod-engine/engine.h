#ifndef Moon64ModEngine
#define Moon64ModEngine

extern "C" {
#include "pc/gfx/gfx_pc.h"
}

void Moon_SaveTexture(TextureData* data, string tex);
TextureData* Moon_GetTexture(string texture);
void Moon_PreInitModEngine();
void Moon_InitModEngine();

void Moon_LoadBaseTexture(char* data, long size, string texture);

#endif