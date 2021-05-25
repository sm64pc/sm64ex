#include "mdebug.h"

#include "moon/achievements/achievements.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/ui/moon-ui-manager.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
#include "pc/pc_main.h"
}

MDebugCategory::MDebugCategory() : MoonCategory("Moon Debug"){
    this->titleKey = false;
    this->catOptions.push_back(new MWValue(22, 57 + (0 * 17), "Trigger Notification",   { .btn = [](){
        Moon::showAchievement(AchievementList::TRIPLE_JUMP);
    }}, false));
}