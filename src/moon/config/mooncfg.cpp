#include "mooncfg.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "moon/utils/moon-env.h"
#include <filesystem>
namespace fs = std::filesystem;

using namespace std;
using json = nlohmann::json;

MoonCFG::MoonCFG(string path, bool relativePath){
    if(relativePath){
        string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
        this->path = cwd.substr(0, cwd.find_last_of("/\\")) + "/" + path;
    } else {
        this->path = path;
    }
    this->reload();
}

vector<string> split (const string &s, char delim) {
    vector<string> result;
    stringstream ss (s);
    string item;
    while (getline (ss, item, delim)) {
        result.push_back (item);
    }
    return result;
}

string MoonCFG::formatNestedKey(string key){
    vector<string> dots = split(key, '.');
    string tmp;
    if(dots.size() > 1)
        for(int i = 0; i < dots.size(); i++){
            tmp += "/" + dots[i];
        }
    else
        tmp = "/"+dots[0];

    return tmp;
}

json MoonCFG::nested(string key){
    vector<string> dots = split(key, '.');
    if(!this->vjson.is_object())
        return this->vjson;
    json gjson = this->vjson.unflatten();

    if(dots.size() > 1){
        for(auto &key : dots){
            if(gjson.contains(key))
                gjson = gjson[key];
        }
        return gjson;
    }

    return gjson[dots[0]];
}

string MoonCFG::getString(string key){
    json n = this->nested(key);
    if(n.is_string())
        return n;
    return "";
}

float MoonCFG::getFloat(string key){
    json n = this->nested(key);
    if(n.is_number_float())
        return n;
    return 0.0f;
}

bool MoonCFG::getBool(string key){
    json n = this->nested(key);
    if(n.is_boolean())
        return n;
    return false;
}

int MoonCFG::getInt(string key){
    json n = this->nested(key);
    if(n.is_number_integer())
        return n;
    return 0;
}

bool MoonCFG::contains(string key){
    return !this->nested(key).is_null();
}

void MoonCFG::setString(string key, string value){
    this->vjson[formatNestedKey(key)] = value;
}

void MoonCFG::setFloat(string key, float value){
    this->vjson[formatNestedKey(key)] = value;
}

void MoonCFG::setBool(string key, bool value){
    this->vjson[formatNestedKey(key)] = value;
}

void MoonCFG::setInt(string key, int value){
    this->vjson[formatNestedKey(key)] = value;
}

void MoonCFG::reload(){
    if(!fs::exists(this->path) || !fs::is_regular_file(this->path)){
        this->isNewInstance = true;
        return;
    }
    std::ifstream ifs(this->path);
    try{
        this->vjson = json::parse(ifs).flatten();
    } catch(...){
        this->vjson = json::object();
    }
}

void MoonCFG::save(){
    if(!fs::exists(this->path)){
        fs::create_directories(fs::path(this->path).parent_path());
    }
    ofstream file(this->path);
    file << this->vjson.unflatten().dump(4);
}