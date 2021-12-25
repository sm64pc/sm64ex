#include "mcheats.h"

#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/cheats.h"
}

MCheatsCategory::MCheatsCategory() : MoonCategory("TEXT_OPT_CHEATS"){
    this->catOptions.push_back(new MWValue(22, 57,  "TEXT_OPT_CHEAT1", {.bvar = &Cheats.EnableCheats }, true));
    this->catOptions.push_back(new MWValue(22, 74,  "TEXT_OPT_CHEAT2", {.bvar = &Cheats.MoonJump     }, true));
    this->catOptions.push_back(new MWValue(22, 91,  "TEXT_OPT_CHEAT3", {.bvar = &Cheats.GodMode      }, true));
    this->catOptions.push_back(new MWValue(22, 108, "TEXT_OPT_CHEAT4", {.bvar = &Cheats.InfiniteLives}, true));
    this->catOptions.push_back(new MWValue(22, 125, "TEXT_OPT_CHEAT5", {.bvar = &Cheats.SuperSpeed   }, true));
    this->catOptions.push_back(new MWValue(22, 142, "TEXT_OPT_CHEAT6", {.bvar = &Cheats.Responsive   }, true));
    this->catOptions.push_back(new MWValue(22, 159, "TEXT_OPT_CHEAT7", {.bvar = &Cheats.ExitAnywhere }, true));
    //this->catOptions.push_back(new MWValue(22, 176, "TEXT_OPT_CHEAT8", {.bvar = &Cheats.HugeMario    }, true));
    //this->catOptions.push_back(new MWValue(22, 193, "TEXT_OPT_CHEAT9", {.bvar = &Cheats.TinyMario    }, true));
}