#ifndef MoonFSAPI
#define MoonFSAPI

#include "moon/mod-engine/interfaces/file-entry.h"
#include <vector>
#include <string>

namespace FSUtils {
    std::string normalize(std::string path);
    std::string joinPath(std::string base, std::string file);
}

enum FileType {
    ZIP,
    DIRECTORY
};

class MoonFS {
public:
    MoonFS(std::string path);
    void open();
    bool exists(std::string path);
    std::vector<std::string> entries();
    std::string read(std::string file);
    std::wstring readWide(std::string file);
    void read(std::string file, EntryFileData *entry);
    std::string getPath();
    void extract(std::string path);
    void close();
protected:
    std::string path;
    FileType type;
};

#endif