#include "mgame.h"

#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
#include "pc/pc_main.h"
}

int languageIdx;
vector<string> lngNames;

MGameCategory::MGameCategory() : MoonCategory("TEXT_OPT_GAME"){
    for (auto &lng : languages) {
        lngNames.push_back(lng->name);
    }
    this->catOptions.push_back(new MWValue(22, 57, "TEXT_OPT_LANGUAGE", {.index = &languageIdx, .values = &lngNames, .callback = [](){
        Moon_SetLanguage(languages[languageIdx]);
    }}, true));
    this->catOptions.push_back(new MWValue(22, 74, "TEXT_OPT_PRECACHE", {.bvar = &configPrecacheRes}, true));
    int exitY;
#ifdef TARGET_SWITCH
    exitY = 108;
    this->catOptions.push_back(new MWValue(22, 91, "TEXT_OPT_SWITCH_HUD", {.bvar = &configSwitchHud}, true));
#else
    exitY = 91;
#endif
    this->catOptions.push_back(new MWValue(22, exitY, "TEXT_EXIT_GAME",    { .btn = game_exit}, true));
}