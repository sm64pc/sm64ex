#ifndef MoonEnvironmentVars
#define MoonEnvironmentVars

#include <string>

namespace MoonInternal {
    void saveEnvironmentVar(std::string key, std::string value);
    std::string getEnvironmentVar(std::string key);
}

#endif