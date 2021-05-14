#ifndef MoonCFG
#define MoonCFG

#include <vector>
#include <string>

namespace Moon {
    extern std::vector<std::string> texturePacks;

    void saveConfig();
    void loadConfig();
}

#endif