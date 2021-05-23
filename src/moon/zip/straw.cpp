#include "straw.h"

#include "moon/libs/miniz/zip_file.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <filesystem>

using namespace std;
using namespace miniz_cpp;
namespace fs = std::filesystem;

bool isDirectory;
zip_file zipFile;

StrawFile::StrawFile(string path){
    this->path = MoonFS::normalize(path);
}

namespace MoonFS {
    string normalize(string path){
        replace(path.begin(), path.end(), '\\', '/');
        return path;
    }

    string joinPath(string base, string file){
        return normalize((fs::path(base) / fs::path(file)).string());
    }
}


void StrawFile::open(){
    if(!(isDirectory = fs::is_directory(this->path)))
        zipFile.load(this->path);
}

bool StrawFile::exists(string path){
    return isDirectory ? fs::exists(MoonFS::joinPath(this->path, path)) : zipFile.has_file(path);
}

vector<string> StrawFile::entries(){
    if(isDirectory) {
        vector<string> fileList;
        for (auto & entry : fs::recursive_directory_iterator(this->path)){
            string path = MoonFS::normalize(entry.path().string());
            fileList.push_back(path.substr(path.find(this->path) + this->path.length() + 1));
        }
        return fileList;
    }

    return zipFile.namelist();
}

string StrawFile::read(string file){
    if(isDirectory){
        std::ifstream in(MoonFS::joinPath(this->path, file), std::ios::in | std::ios::binary);
        if (in)
            return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }

    return zipFile.read(file);
}

void StrawFile::read(string file, TextureFileEntry *entry){
    if(isDirectory){
        char *data;
        long size;
        FILE* f = fopen(MoonFS::joinPath(this->path, file).c_str(), "rb");

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