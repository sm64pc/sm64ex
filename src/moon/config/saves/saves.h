#ifndef MoonSaveFileAPI
#define MoonSaveFileAPI
#ifdef __cplusplus

#include <string>

namespace MoonInternal {
    void setupSaveEngine(std::string state);
}
#else
void writeSaveFile(int saveIndex);
void readSaveFile(int saveIndex);
void eraseSaveFile(int fileIndex);
#endif
#endif