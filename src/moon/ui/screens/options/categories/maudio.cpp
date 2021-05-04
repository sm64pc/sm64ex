#include "maudio.h"

#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

MAudioCategory::MAudioCategory() : MoonCategory("TEXT_OPT_AUDIO"){
    this->catOptions.push_back(new MWValue(22, 57,  "TEXT_OPT_MVOLUME",  {.ivar = (int*)&configMasterVolume,  .max = 127.0f, .min = .0f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 74,  "TEXT_OPT_MUSVOLUME", {.ivar = (int*)&configMusicVolume,  .max = 127.0f, .min = .0f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 91,  "TEXT_OPT_SFXVOLUME", {.ivar = (int*)&configSfxVolume,    .max = 127.0f, .min = .0f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 108, "TEXT_OPT_ENVVOLUME", {.ivar = (int*)&configEnvVolume,    .max = 127.0f, .min = .0f, .step = 1.0f}, true));
}