#include "mgame.h"

#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/ui/moon-ui-manager.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
#include "pc/pc_main.h"
}

vector<wstring> lngNames;
vector<wstring> modes = {
    L"Auto", L"Low", L"Disabled"
};

MGameCategory::MGameCategory() : MoonCategory("TEXT_OPT_GAME"){
    for (auto &lng : Moon::languages) {
        lngNames.push_back(lng->name);
    }

    this->catOptions.push_back(new MWValue(22, 57, "TEXT_OPT_LANGUAGE",   {.index = reinterpret_cast<int*>(&configLanguage), .values = &lngNames, .callback = [](){
        if( configLanguage < Moon::languages.size())
            Moon::setCurrentLanguage(Moon::languages[configLanguage]);
    }}, true));
    this->catOptions.push_back(new MWValue(22, 74, "TEXT_OPT_PRECACHE",   {.bvar = &configPrecacheRes}, true));
#ifdef TARGET_SWITCH
    this->catOptions.push_back(new MWValue(22, 91, "TEXT_OPT_SWITCH_HUD", {.bvar = &configSwitchHud}, true));
#endif
    this->catOptions.push_back(new MWValue(22, 0, "Level of detail",  {.index = reinterpret_cast<int*>(&configLODMode), .values = &modes }, false));
    this->catOptions.push_back(new MWValue(22, 0, "Achievements",   { .btn = [](){
        MoonChangeUI(2);
    }}, false));
    this->catOptions.push_back(new MWValue(22, 0, "Addons",   { .btn = [](){
        MoonChangeUI(1);
    }}, false));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_EXIT_GAME",   { .btn = game_exit}, true));
}