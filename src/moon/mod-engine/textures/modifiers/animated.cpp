#include "animated.h"
#include "moon/mod-engine/hooks/hook.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

extern "C" {
#include "moon/utils/moon-gfx.h"
}

using json = nlohmann::json;
using namespace std;

map<string, AnimatedEntry*> textures;

void AnimatedModifier::onInit(){
    Moon::registerHookListener({.hookName = TEXTURE_BIND, .callback = [](HookCall call){
        char* *hookTexture = reinterpret_cast<char**>(call.baseArgs["texture"]);
        string texName = string(*hookTexture);
        if(textures.find(texName) != textures.end()){
            AnimatedEntry* entry = textures[texName];
            Frame *frame = entry->frames[entry->lastFrame];

            if(moon_get_milliseconds() >= entry->lastTime + frame->delay){
                int maxFrames = entry->frames.size() - 1;
                bool reachMax = (entry->lastFrame < maxFrames);
                if(entry->bounce){
                    if(entry->lastFrame >= maxFrames)
                        entry->lastBounce = true;
                    else if(entry->lastFrame <= 0)
                        entry->lastBounce = false;
                }

                entry->lastFrame += entry->bounce ? entry->lastBounce ? -1 : 1 : (reachMax ? 1 : -entry->lastFrame);
                entry->lastTime = moon_get_milliseconds();
                frame = entry->frames[entry->lastFrame];
            }

            (*hookTexture) = const_cast<char*>(frame->path.c_str());
        }
    }});
}

void AnimatedModifier::onLoad(std::string texture, json data){
    cout << "Called on load" << endl;
    if(textures.find(texture) == textures.end()){
        if(!(data.contains("frames") && data.contains("bounce"))) return;
        cout << "Found animated entry: " << texture << " with " << data["frames"].size() << " length" << endl;
        AnimatedEntry *entry = new AnimatedEntry();
        entry->advancedMode = data.contains("advancedMode") ? (bool) data["advancedMode"] : false;

        entry->bounce = data["bounce"];
        cout << "Registered animated frames " << data["frames"].size() << endl;
        // cout << "File " << frame["path"] << " Time " << frame["delay"];
        for(auto &frame : data["frames"]){
            if(!entry->advancedMode && data.contains("delay"))
                entry->frames.push_back( new Frame({.delay = data["delay"], .path = frame}));
            else
                entry->frames.push_back( new Frame({.delay = frame["delay"], .path = frame["path"]}));
        }
        textures[texture] = entry;
    }
}

void AnimatedModifier::onRelease(){
    textures.clear();
}

std::string AnimatedModifier::getKey(){
    return "animated";
}