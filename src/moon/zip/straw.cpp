#include "straw.h"

#include "moon/libs/miniz/zip_file.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;
using namespace miniz_cpp;

bool isDirectory;

#ifdef __MINGW32__
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

zip_file zipFile;

void StrawFile::open(){
    if(!(isDirectory = fs::is_directory(this->path)))
        zipFile.load(this->path);
}

bool StrawFile::exists(string path){
    return isDirectory ? fs::exists(this->path + SEPARATOR + path) : zipFile.has_file(path);
}

vector<string> StrawFile::entries(){
    if(isDirectory) {
        vector<string> fileList;
        for (auto & entry : fs::recursive_directory_iterator(this->path)){
        #ifndef TARGET_SWITCH
            fileList.push_back(fs::relative(entry, this->path).string());
        #else
            fileList.push_back(entry.path().string());
        #endif
        }
        return fileList;
    }

    return zipFile.namelist();
}

string StrawFile::read(string file){
    if(isDirectory){
        std::ifstream in(this->path + SEPARATOR + file, std::ios::in | std::ios::binary);
        if (in)
            return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }

    return zipFile.read(file);
}

void StrawFile::read(string file, TextureFileEntry *entry){
    if(isDirectory){
        char *data;
        long size;
        FILE* f = fopen((this->path + SEPARATOR + file).c_str(), "r");

        fseek(f, 0, SEEK_END);
        size = ftell(f);
        rewind(f);

        data = new char[size];
        fread(data, sizeof(char), size, f);

        entry->data = data;
        entry->size = size;

        fclose(f);
        return;
    }
    zipFile.read_texture(file, &entry);
}

string StrawFile::getPath(){
    return this->path;
}