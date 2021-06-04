#include "addons-view.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/mod-engine/engine.h"
#include "moon/achievements/achievements.h"
#include <cstring>
#include <algorithm>

using namespace std;

extern "C" {
#include "sm64.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"
}

int scrollModifier = 0;

int focusFlag;
int focusRange = 80;
float focusAnim = focusRange / 2;

void MoonAddonsScreen::Init(){
    this->scrollIndex = 0;
    scrollModifier = 0;
}

void MoonAddonsScreen::Mount(){

}

bool dispatched;

void MoonAddonsScreen::changeScroll(int idx){
    if(idx < 0){
        if(this->scrollIndex > 0){
            if(scrollModifier > 0 && this->scrollIndex == scrollModifier)
                scrollModifier--;
            this->scrollIndex--;
            return;
        }
        this->scrollIndex = registeredAchievements.size() - 1;
        scrollModifier = this->scrollIndex - 4;
        return;
    }
    if(this->scrollIndex < registeredAchievements.size() - 1){
        if(this->scrollIndex > 3 && !((this->scrollIndex - scrollModifier) % 4)) scrollModifier++;
        this->scrollIndex++;
        return;
    }
    this->scrollIndex = 0;
    scrollModifier = 0;
}

void MoonAddonsScreen::Update(){
    float yStick = GetStickValue(MoonButtons::U_STICK, false);
    if(yStick > 0) {
        if(dispatched) return;
        MoonAddonsScreen::changeScroll(-1);
        dispatched = true;
    }
    if(yStick < 0) {
        if(dispatched) return;
        MoonAddonsScreen::changeScroll(1);
        dispatched = true;
    }
    if(!yStick)
        dispatched = false;

    if(IsBtnPressed(MoonButtons::A_BTN)) {

    }
    if(IsBtnPressed(MoonButtons::B_BTN)) {
        MoonChangeUI(0);
    }
    MoonScreen::Update();
}

static std::string cropTxt(string txt, int length){
    int currLngt = min((int) txt.length(), length);
    string desc = txt.substr(0, currLngt);
    desc.erase(find_if(desc.rbegin(), desc.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), desc.end());
    return currLngt >= length ? desc + " ..." : desc;
}

char *strdup(const char *src_str) noexcept {
    char *new_str = new char[std::strlen(src_str) + 1];
    std::strcpy(new_str, src_str);
    return new_str;
}

void MoonAddonsScreen::Draw(){
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

    int packAmount = registeredAchievements.size();
    int maxPacks = 5;
    int iMod = scrollModifier;

    for(int i = 0; i < min(packAmount, maxPacks); i++){
        int index = i + iMod;

        if(index > packAmount - 1){
            this->scrollIndex = 0;
            scrollModifier = 0;
            cout << "Triggered overflow, coming back to 0" << endl;
            return;
        }

        auto &addon = registeredAchievements[registeredAchievements.];

        if(addon == NULL) return;

        bool selected = (i + iMod) == this->scrollIndex;
        int itemWidth = boxWidth - (selected ? 15 : 0);

        MoonDrawRectangle(35, 45 + (i * 35), itemWidth - 20, 31, (i + iMod) == this->scrollIndex && !selected ? focusColor : (Color){0, 0, 0, 100}, true);
        string pathPrefix = "mod-icons://";
        string iconPath = pathPrefix.append(addon->name);
        char* parsed = strdup(iconPath.c_str());
        if(parsed != nullptr)
            MoonDrawTexture(35, 45 + (i * 35), 30, 30, parsed);
        MoonDrawText(35 + 26, 46 + (i * 35), to_string(i + iMod + 1), 0.5, {255, 255, 255, 255}, true, true);

        MoonDrawText(70, 45 + (i * 35) + 3, cropTxt(addon->name, 37), 0.8, {255, 255, 255, 255}, true, true);

        int maxDesc = 37;
        int currLngt = min((int) addon->description.length(), maxDesc);
        MoonDrawText(70, 45 + (i * 35) + 16, cropTxt(addon->description, 37), 0.8, {255, 255, 255, 255}, true, true);

        string rawVer = to_string(addon->version);
        string version = "v"+rawVer.substr(0, rawVer.find(".")+2);

        MoonDrawText(itemWidth + 13 - MoonGetTextWidth(version, 0.5, false),       45 + (i * 35) + 2,      version,       0.5, {255, 255, 255, 255}, true, true);
        MoonDrawText(itemWidth + 13 - MoonGetTextWidth(addon->authors[0], 0.5, false), 45 + (i * 35) + 22, addon->authors[0], 0.5, {255, 255, 255, 255}, true, true);
    }

    MoonScreen::Draw();
}