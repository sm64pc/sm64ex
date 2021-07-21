#ifndef Moon64ModEngine
#define Moon64ModEngine

#include "moon/mod-engine/interfaces/bit-module.h"
#include <vector>
#include <string>

extern "C" {
#include "pc/gfx/gfx_pc.h"
}
namespace Moon {
    extern std::vector<BitModule*> addons;
    extern std::map<std::string, EntryFileData*> fonts;
    void loadAddon(std::string addonPath);
}

namespace MoonInternal {
    void setupModEngine( std::string state );
}


#endif