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
    #ifndef TARGET_SWITCH
    this->catOptions.push_back(new MWValue(22, 57,  "TEXT_OPT_FSCREEN",   {.bvar = &configWindow.fullscreen}, true));
    this->catOptions.push_back(new MWValue(22, 74,  "TEXT_OPT_VSYNC",     {.bvar = &configWindow.vsync}, true));
    #endif
    this->catOptions.push_back(new MWValue(22, 91,  "TEXT_OPT_TEXFILTER", {.index = (int*)&configFiltering, .values = &texNames}, true));
    this->catOptions.push_back(new MWValue(22, 108, "TEXT_OPT_HUD",       {.bvar = &configHUD}, true));
    #ifndef TARGET_SWITCH
    this->catOptions.push_back(new MWValue(22, 125, "TEXT_OPT_RESETWND",  {.btn = [](){
        configWindow.reset = true;
        configWindow.settings_changed = true;
    }}, true));
    this->catOptions.push_back(new MWValue(22, 142, "TEXT_OPT_APPLY",     {.btn = [](){
        configWindow.settings_changed = true;
    }}, true));
    #endif
}