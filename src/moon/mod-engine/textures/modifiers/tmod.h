#ifndef ModEngineTextureModifier
#define ModEngineTextureModifier

#include "moon/libs/nlohmann/json.hpp"
#include <string>

class TextureModifier {
public:
    virtual void onInit() = 0;
    virtual void onLoad(std::string texture, nlohmann::json data) = 0;
    virtual void onRelease() = 0;
    virtual std::string getKey() = 0;
};

#endif