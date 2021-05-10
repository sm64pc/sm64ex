#include "moon-env.h"
#include <map>

using namespace std;

map<string, string> environmentVars;

namespace MoonInternal {
    void saveEnvironmentVar(std::string key, std::string value){
        environmentVars.insert(pair<string, string>(key, value));
    }
    std::string getEnvironmentVar(std::string key){
        return environmentVars[key];
    }
}