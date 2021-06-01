#ifndef MoonCFGApi
#define MoonCFGApi

#include <vector>
#include <string>
#include "moon/libs/nlohmann/json.hpp"

class MoonCFG {
protected:
    std::string path;
public:
    MoonCFG(std::string path);

    nlohmann::json vjson;
    nlohmann::json nested(std::string key);
    std::string getString(std::string key);
    float getFloat(std::string key);
    bool getBool(std::string key);
    int getInt(std::string key);

    void setString(std::string key, std::string value);
    void setFloat(std::string key, float value);
    void setBool(std::string key, bool value);
    void setInt(std::string key, int value);

    void reload();
    void save();
};

#endif