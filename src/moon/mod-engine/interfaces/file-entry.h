#ifndef Moon64FileEntry
#define Moon64FileEntry
#include <string>

struct TextureFileEntry {
    std::string path;
    long size;
    char* data;
};

#endif