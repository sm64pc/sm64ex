#include "moonfs.h"

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

MoonFS::MoonFS(string path){
    this->path = FSUtils::normalize(path);
    this->type = fs::is_directory(this->path) ? DIRECTORY : ZIP;
}

namespace FSUtils {
    string normalize(string path){
        replace(path.begin(), path.end(), '\\', '/');
        return path;
    }

    string joinPath(string base, string file){
        return normalize((fs::path(base) / fs::path(file)).string());
    }
}


void MoonFS::open(){
    switch(this->type){
        case ZIP:
            zipFile.load(this->path);
            break;
    }

}

bool MoonFS::exists(string path){
    switch(this->type){
        case DIRECTORY:
            return fs::exists(FSUtils::joinPath(this->path, path));
        case ZIP:
            return zipFile.has_file(path);
    }
}

vector<string> MoonFS::entries(){
    switch(this->type){
        case DIRECTORY: {
            vector<string> fileList;
            for (auto & entry : fs::recursive_directory_iterator(this->path)){
                string path = FSUtils::normalize(entry.path().string());
                fileList.push_back(path.substr(path.find(this->path) + this->path.length() + 1));
            }
            return fileList;
        }
        case ZIP:
            return zipFile.namelist();
    }
}

string MoonFS::read(string file){
    switch(this->type){
        case DIRECTORY: {
            std::ifstream in(FSUtils::joinPath(this->path, file), std::ios::in | std::ios::binary);
            if (in)
                return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
        }
        case ZIP:
            return zipFile.read(file);
    }
}

void MoonFS::read(string file, TextureFileEntry *entry){
     switch(this->type){
        case DIRECTORY: {
            char *data;
            long size;
            FILE* f = fopen(FSUtils::joinPath(this->path, file).c_str(), "rb");

            if(f == NULL){
                cout << "ERROR: Failed to open file " << file << endl;
                return;
            }
            fseek(f, 0, SEEK_END);
            size = ftell(f);
            rewind(f);

            data = new char[size];
            fread(data, sizeof(char), size, f);

            entry->data = data;
            entry->size = size;

            fclose(f);
            break;
        }
        case ZIP:
            zipFile.read_texture(file, &entry);
            break;
    }
}

string MoonFS::getPath(){
    return this->path;
}