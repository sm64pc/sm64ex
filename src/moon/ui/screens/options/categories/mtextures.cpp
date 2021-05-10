#include "mtextures.h"

#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/mod-engine/engine.h"
#include "moon/mod-engine/textures/mod-texture.h"
#include <algorithm>

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

std::vector<int> order;

MWValue *tmpButton;

MTexturesCategory::MTexturesCategory() : MoonCategory("Textures"){
    this->titleKey = false;
    for(int i = 0; i < Moon::addons.size(); i++) order.push_back(i);

    this->catOptions.push_back(tmpButton = new MWValue(22, 57, "Randomize packs", {.btn = [](){
        tmpButton->title = "Randomize on progress";
        std::random_shuffle ( order.begin(), order.end() );
        MoonInternal::buildTextureCache(order);
        tmpButton->title = "Randomize packs";
    }}, false));
}