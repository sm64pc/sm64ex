#include "mtextures.h"

#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/mod-engine/engine.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

MTexturesCategory::MTexturesCategory() : MoonCategory("TEXT_OPT_AUDIO"){
    int idx = 0;
    // for(auto &addon : addons){
        // this->catOptions.push_back(new MWValue(22, 57 + (17 * idx), addon->name, {.bvar = &addon->enabled}, false));
    //  idx++;
    // }
    this->catOptions.push_back(new MWValue(22, 57, "Load main pack", {.btn = [](){
        Moon_TextFlyLoad(0);
    }}, false));
    this->catOptions.push_back(new MWValue(22, 57 + (17 * 1), "Load minecraft pack", {.btn = [](){
        Moon_TextFlyLoad(1);
    }}, false));
    this->catOptions.push_back(new MWValue(22, 57 + (17 * 2), "Load owo pack", {.btn = [](){
        Moon_TextFlyLoad(2);
    }}, false));
    this->catOptions.push_back(new MWValue(22, 57 + (17 * 3), "Load Render96 pack", {.btn = [](){
        Moon_TextFlyLoad(2);
    }}, false));
}