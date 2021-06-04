#ifndef Moon64FileEntry
#define Moon64FileEntry
#include <string>

struct EntryFileData {
    std::string path;
    long size;
    char* data;
};

#endif