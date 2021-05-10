#include "straw.h"

#include "moon/libs/miniz/zip_file.hpp"
using namespace std;
using namespace miniz_cpp;

zip_file zipFile;

void StrawFile::open(){
    zipFile.load(this->path);
}
bool StrawFile::exists(string path){
    return zipFile.has_file(path);
}
vector<string> StrawFile::entries(){
    return zipFile.namelist();
}
string StrawFile::read(string file){
    return zipFile.read(file);
}
void StrawFile::read(string file, TextureFileEntry *entry){
    zipFile.read_texture(file, &entry);
}

string StrawFile::getPath(){
    return this->path;
}