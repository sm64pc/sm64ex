#ifndef ModEngineSoundModule
#define ModEngineSoundModule

#include "moon/mod-engine/interfaces/file-entry.h"
#include "moon/mod-engine/interfaces/bit-module.h"
#include "moon/libs/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <map>

namespace Moon {
    void saveAddonSound(BitModule *addon, std::string soundPath, EntryFileData* data);
}

namespace MoonInternal {
    EntryFileData *getSoundData(const char *fullpath);
    void buildSoundCache(std::vector<int> order);
    void* loadSoundData(const char* fullpath);
    void buildAudioCache(std::vector<int> order);
    void resetSoundSystem();
}

#endif