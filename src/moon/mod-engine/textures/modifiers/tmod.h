#ifndef ModEngineTextureModifier
#define ModEngineTextureModifier

#include "moon/libs/nlohmann/json.hpp"
#include <string>

class TextureModifier {
public:
    virtual void onInit(){}
    virtual void onLoad(std::string texture, nlohmann::json data){}
    // virtual void onRelease(){}
    virtual std::string getKey(){}
};

#endif