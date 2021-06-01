#ifndef MoonCFGApi
#define MoonCFGApi

#include <vector>
#include <string>
#include <sstream>
#include "moon/libs/nlohmann/json.hpp"

class MoonCFG {
protected:
    std::string path;
public:
    MoonCFG(std::string path);

    nlohmann::json vjson;
    nlohmann::json nested(std::string key);
    std::string formatNestedKey(std::string key);
    std::string getString(std::string key);
    float getFloat(std::string key);
    bool getBool(std::string key);
    int getInt(std::string key);
    template< typename T > T* getArray(std::string key) {
        return this->nested(key).get<std::vector<T>>();
    };

    void setString(std::string key, std::string value);
    void setFloat(std::string key, float value);
    void setBool(std::string key, bool value);
    void setInt(std::string key, int value);
    template< typename T > void setArray(std::string key, std::vector<T> array) {
        for(int i = 0; i < array.size(); i++)
            this->vjson[formatNestedKey(key)+"/"+std::to_string(i)] = array[i];
    };

    void reload();
    void save();
};

#endif