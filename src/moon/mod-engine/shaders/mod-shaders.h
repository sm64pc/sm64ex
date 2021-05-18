#ifndef ModEngineShadersModule
#define ModEngineShadersModule
#ifdef __cplusplus

#include "moon/mod-engine/interfaces/bit-module.h"
#include "moon/mod-engine/interfaces/shader-entry.h"
#include <string>
#include <vector>
#include <map>

extern "C" {
#include "pc/gfx/gfx_pc.h"
}

namespace Moon {
    void saveAddonShader(BitModule *addon, std::string shaderID, std::string shaderData, ShaderType type);
};

#else

void compileShaders(unsigned int shaderProgram, void (*glCallback)(unsigned int shaderProgram, const char** sources, unsigned long* length));

#endif
#endif