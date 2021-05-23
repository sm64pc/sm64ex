#include "straw.h"

#include "moon/libs/miniz/zip_file.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;
using namespace miniz_cpp;

bool isDirectory;

string normalize(string path){
    replace(path.begin(), path.end(), '\\', '/');
    return path;
}

string joinPath(string base, string file){
    return normalize((fs::path(base) / fs::path(file)).string());
}

StrawFile::StrawFile(string path){
    this->path = normalize(path);
}

zip_file zipFile;

void StrawFile::open(){
    if(!(isDirectory = fs::is_directory(this->path)))
        zipFile.load(this->path);
}

bool StrawFile::exists(string path){
    return isDirectory ? fs::exists(joinPath(this->path, path)) : zipFile.has_file(path);
}

vector<string> StrawFile::entries(){
    if(isDirectory) {
        vector<string> fileList;
        for (auto & entry : fs::recursive_directory_iterator(this->path)){
            string path = normalize(entry.path().string());
            fileList.push_back(path.substr(path.find(this->path) + this->path.length() + 1));
        }
        return fileList;
    }

    return zipFile.namelist();
}

string StrawFile::read(string file){
    if(isDirectory){
        std::ifstream in(joinPath(this->path, file), std::ios::in | std::ios::binary);
        if (in)
            return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }

    return zipFile.read(file);
}

void StrawFile::read(string file, TextureFileEntry *entry){
    if(isDirectory){
        char *data;
        long size;
        FILE* f = fopen(joinPath(this->path, file).c_str(), "rb");

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