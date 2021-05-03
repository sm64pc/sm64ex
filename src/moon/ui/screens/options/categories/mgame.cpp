#include "mgame.h"

#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

int languageIdx;
vector<string> lngNames;
MWValue * lngSwitch;

MGameCategory::MGameCategory() : MoonCategory("TEXT_OPT_GAME"){
    for (auto &lng : languages) {
        lngNames.push_back(lng->name);
    }
    this->catOptions.push_back(lngSwitch = new MWValue(22, 57, Moon_GetKey("TEXT_OPT_LANGUAGE"), {.index = &languageIdx, .values = &lngNames, .callback = [](){
        Moon_SetLanguage(languages[languageIdx]);
        lngSwitch->title = Moon_GetKey("TEXT_OPT_LANGUAGE");
    }}));
    this->catOptions.push_back(new MWValue(22, 74, Moon_GetKey("TEXT_OPT_PRECACHE"), {.bvar = &configPrecacheRes}));
#ifdef TARGET_SWITCH
    this->catOptions.push_back(new MWValue(22, 91, Moon_GetKey("TEXT_OPT_SWITCH_HUD"), {.bvar = &configSwitchHud}));
#endif
}