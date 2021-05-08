#include "mtextures.h"

#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/mod-engine/engine.h"
#include <algorithm>

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

std::vector<int> order = {
    0, 2, 1
};

MWValue *tmpButton;

MTexturesCategory::MTexturesCategory() : MoonCategory("TEXT_OPT_AUDIO"){
    int idx = 0;
    // for(auto &addon : addons){
        // this->catOptions.push_back(new MWValue(22, 57 + (17 * idx), addon->name, {.bvar = &addon->enabled}, false));
    //  idx++;
    // }
    this->catOptions.push_back(tmpButton = new MWValue(22, 57, "Randomize packs", {.btn = [](){
        tmpButton->title = "Randomize on progress";
        std::random_shuffle ( order.begin(), order.end() );
        Moon_TestRebuildOrder(order);
        tmpButton->title = "Randomize packs";
    }}, false));
    // this->catOptions.push_back(new MWValue(22, 57 + (17 * 1), "Load minecraft pack", {.btn = [](){
    //     Moon_TextFlyLoad(1);
    // }}, false));
    // this->catOptions.push_back(new MWValue(22, 57 + (17 * 2), "Load owo pack", {.btn = [](){
    //     Moon_TextFlyLoad(2);
    // }}, false));
    // this->catOptions.push_back(new MWValue(22, 57 + (17 * 3), "Load Render96 pack", {.btn = [](){
    //     Moon_TextFlyLoad(2);
    // }}, false));
}