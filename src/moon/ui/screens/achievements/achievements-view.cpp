#include "achievements-view.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/mod-engine/engine.h"
#include "moon/achievements/achievements.h"
#include <cstring>
#include <algorithm>

using namespace std;

extern "C" {
#include "pc/platform.h"
#include "sm64.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"
}

void MoonAchievementsScreen::Init(){
    this->scrollIndex = 0;
    scrollModifier = 0;
}

void MoonAchievementsScreen::Mount(){

}


bool sortByNID(const string &a, const string &b) {
    return registeredAchievements[a]->sortId < registeredAchievements[b]->sortId;
}

vector<string> getAchievementKeys(){
    std::vector<string> keys;
    for(auto it = registeredAchievements.begin(); it != registeredAchievements.end(); ++it)
        if(it->first != "achievement.cheater")
            keys.push_back(it->first);
    sort(keys.begin(), keys.end(), sortByNID);
    return keys;
}

void MoonAchievementsScreen::changeScroll(int idx){
    int size = getAchievementKeys().size();
    if(idx < 0){
        if(this->scrollIndex > 0){
            if(scrollModifier > 0 && this->scrollIndex == scrollModifier)
                scrollModifier--;
            this->scrollIndex--;
            return;
        }
        this->scrollIndex = size - 1;
        scrollModifier = this->scrollIndex - 4;
        return;
    }
    if(this->scrollIndex < size - 1){
        if(this->scrollIndex > 3 && !((this->scrollIndex - scrollModifier) % 4)) scrollModifier++;
        this->scrollIndex++;
        return;
    }
    this->scrollIndex = 0;
    scrollModifier = 0;
}

void MoonAchievementsScreen::Update(){
    float yStick = GetStickValue(MoonButtons::U_STICK, false);
    if(yStick > 0) {
        if(dispatched) return;
        MoonAchievementsScreen::changeScroll(-1);
        dispatched = true;
    }
    if(yStick < 0) {
        if(dispatched) return;
        MoonAchievementsScreen::changeScroll(1);
        dispatched = true;
    }
    if(!yStick)
        dispatched = false;

    if(IsBtnPressed(MoonButtons::B_BTN)) {
        MoonChangeUI(0);
    }

    if(!(gGlobalTimer % 20))
        stickAnim = !stickAnim;

    MoonScreen::Update();
}

void MoonAchievementsScreen::Draw(){
    string curTitle = "Achievements";
    float step = 1.5;

    if(focusAnim >= focusRange)
        focusFlag = 1;
    else if (focusAnim <= focusRange / 2)
        focusFlag = 0;

    focusAnim += step * (focusFlag ? -1 : 1);

    int boxWidth = SCREEN_WIDTH - 50;
    int boxHeight = GetScreenHeight() * 0.8;

    float txtWidth = MoonGetTextWidth(curTitle, 1.0, true);

    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 10, curTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 35, boxWidth, boxHeight, {0, 0, 0, 100}, true);

    Color focusColor = {255, 255, 255, 40 + focusAnim};

    vector<string> achievementList = getAchievementKeys();

    int packAmount = achievementList.size();
    int maxPacks = 5;
    int iMod = scrollModifier;

    for(int i = 0; i < min(packAmount, maxPacks); i++){
        int index = i + iMod;

        if(index > packAmount - 1){
            this->scrollIndex = 0;
            scrollModifier = 0;
            return;
        }

        auto &achievement = registeredAchievements[achievementList[index]];

        if(achievement == NULL) return;

        bool selected = (i + iMod) == this->scrollIndex;
        int itemWidth = boxWidth - 0;

        MoonDrawRectangle(35, 45 + (i * 35), itemWidth - 20, 31, selected ? focusColor : (Color){0, 0, 0, 100}, true);
        bool isUnlocked = find_if(entries.begin(), entries.end(),  [&cae = achievement] (auto &m) -> bool { return cae->id == m->achievement->id; }) != entries.end();
        string iconPath = isUnlocked ? achievement->icon : achievement->lockedIcon;

        char* parsed = sys_strdup(iconPath.c_str());
        if(parsed != nullptr){
            MoonDrawTexture  (35, 45 + (i * 35), 30, 30, parsed);
        }

        MoonDrawText(70, 45 + (i * 35) + 3, achievement->title, 0.8, {255, 255, 255, 255}, true, true);

        int maxDesc = 37;
        int currLngt = min((int) achievement->description.length(), maxDesc);
        MoonDrawText(70, 45 + (i * 35) + 16, achievement->description, 0.8, {255, 255, 255, 255}, true, true);

        string basePath = "textures/moon/controller/";
        basePath.append(stickAnim ? "stick-down.rgba16" : "stick-up.rgba16");

        MoonDrawButton(5, GetScreenHeight() - 24, "Move", basePath, 16, 0, false, false);
    }

    MoonScreen::Draw();
}