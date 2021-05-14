#ifndef AnimatedTextureModifier
#define AnimatedTextureModifier

#include "moon/mod-engine/interfaces/bit-module.h"
#include "tmod.h"
#include <vector>
#include <string>

struct Frame {
    long delay;
    std::string path;
};

struct AnimatedEntry {
    std::vector<Frame*> frames;
    bool bounce;
    bool advancedMode;

    int lastFrame;
    bool lastBounce;
    long long lastTime;
};

class AnimatedModifier : public TextureModifier {
public:
   void onInit();
   void onLoad(std::string texture, nlohmann::json data);
   void onRelease();
   std::string getKey();
};

#endif