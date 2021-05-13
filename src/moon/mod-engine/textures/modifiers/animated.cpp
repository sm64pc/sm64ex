#include "animated.h"
#include "moon/mod-engine/hooks/hook.h"
#include <sys/time.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;
using namespace std;

map<string, AnimatedEntry*> textures;

long long getMilliseconds(){
    struct timeval te;
    gettimeofday(&te, NULL);
    return te.tv_sec*1000LL + te.tv_usec/1000;
}

void AnimatedModifier::onInit(){
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call){
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(textures.find(texName) != textures.end()){
            AnimatedEntry* entry = textures[texName];

            if(getMilliseconds() >= entry->lastTime + entry->delay){
                int maxFrames = entry->frames.size() - 1;
                bool reachMax = (entry->lastFrame < entry->frames.size() - 1);
                if(entry->bounce){
                    if(entry->lastFrame >= maxFrames)
                        entry->lastBounce = true;
                    else if(entry->lastFrame <= 0)
                        entry->lastBounce = false;
                }

                entry->lastFrame += entry->bounce ? entry->lastBounce ? -1 : 1 : (reachMax ? 1 : -entry->lastFrame);
                entry->lastTime = getMilliseconds();
            }

            (*hookTexture) = const_cast<char*>(entry->frames[entry->lastFrame].c_str());
        }
        return false;
    }});
}

void AnimatedModifier::onLoad(std::string texture, json data){
    if(textures.find(texture) == textures.end()){
        cout << data.contains("frames") << " " << data.contains("delay") << " " << data.contains("bounce") << endl;
        if(!(data.contains("frames") && data.contains("delay") && data.contains("bounce"))) return;
        cout << "Found animated entry: " << texture << " with " << data["frames"].size() << " length" << endl;
        textures[texture] = new AnimatedEntry({
            .frames = data["frames"],
            .delay = data["delay"],
            .bounce = data["bounce"]
        });
    }
}

std::string AnimatedModifier::getKey(){
    return "animated";
}