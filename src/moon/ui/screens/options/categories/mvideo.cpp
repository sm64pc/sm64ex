#include "mvideo.h"

#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

int texIdx;
vector<string> texNames;

MVideoCategory::MVideoCategory() : MoonCategory("TEXT_OPT_VIDEO"){
    string filters[] = {
        "TEXT_OPT_NEAREST",
        "TEXT_OPT_LINEAR",
        "TEXT_OPT_THREEPT"
    };
    for (auto &tex : filters) {
        texNames.push_back(Moon_GetKey(tex));
    }
    this->catOptions.push_back(new MWValue(22, 57,  Moon_GetKey("TEXT_OPT_FSCREEN"),   {.bvar = &configWindow.fullscreen}));
    this->catOptions.push_back(new MWValue(22, 74,  Moon_GetKey("TEXT_OPT_VSYNC"),     {.bvar = &configWindow.vsync}));
    this->catOptions.push_back(new MWValue(22, 91,  Moon_GetKey("TEXT_OPT_TEXFILTER"), {.index = (int*)&configFiltering, .values = &texNames}));
    this->catOptions.push_back(new MWValue(22, 108, Moon_GetKey("TEXT_OPT_HUD"),       {.bvar = &configHUD}));
    this->catOptions.push_back(new MWValue(22, 125, Moon_GetKey("TEXT_OPT_RESETWND"),  {.btn = [](){
        configWindow.reset = true;
        configWindow.settings_changed = true;
    }}));
    this->catOptions.push_back(new MWValue(22, 142, Moon_GetKey("TEXT_OPT_APPLY"),     {.btn = [](){
        configWindow.settings_changed = true;
    }}));
}