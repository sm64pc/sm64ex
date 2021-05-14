#include "addons-view.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/mod-engine/engine.h"
#include "moon/mod-engine/textures/mod-texture.h"
#include <cstring>

using namespace std;

extern "C" {
#include "sm64.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"
}

BitModule* currentPack;
vector<BitModule*> texturePackList;

int currentSubItem = 0;
int focusFlag;
int focusRange = 80;
float focusAnim = focusRange / 2;

enum ItemButtons{
    UP,
    DOWN,
    TOGGLE
};

void MoonAddonsScreen::Init(){
    texturePackList.clear();
    this->scrollIndex = 0;
    currentPack = NULL;
    copy(Moon::addons.begin(), Moon::addons.end(), back_inserter(texturePackList));
    reverse(texturePackList.begin(), texturePackList.end());
}

void MoonAddonsScreen::Mount(){

}

bool dispatched;

void rebuildTextureCache(){
    vector<int> order;
    for(auto &addon : texturePackList){
        vector<BitModule*>::iterator itr = find(Moon::addons.begin(), Moon::addons.end(), addon);
        order.push_back(distance(Moon::addons.begin(), itr));
    }
    reverse(order.begin(), order.end());
    MoonInternal::buildTextureCache(order);
}

void MoonAddonsScreen::Update(){
    float yStick = GetStickValue(MoonButtons::U_STICK, false);
    if(yStick > 0) {
        if(dispatched) return;
        if(currentPack != NULL){
            if(currentSubItem > 0)
                currentSubItem--;
            else
                currentSubItem = 2;
            dispatched = true;
            return;
        }
        if(this->scrollIndex > 0)
            this->scrollIndex--;
        else
            this->scrollIndex = texturePackList.size() - 1;
        dispatched = true;
    }
    if(yStick < 0) {
        if(dispatched) return;
        if(currentPack != NULL){
            if(currentSubItem < 2)
                currentSubItem++;
            else
                currentSubItem = 0;
            dispatched = true;
            return;
        }
        if(this->scrollIndex < texturePackList.size() - 1)
            this->scrollIndex++;
        else
            this->scrollIndex = 0;
        dispatched = true;
    }
    if(!yStick)
        dispatched = false;

    if(IsBtnPressed(MoonButtons::A_BTN)) {
        if(currentPack != NULL){
            switch(currentSubItem){
                case ItemButtons::UP:
                    if(this->scrollIndex > 0){
                        std::swap(texturePackList[this->scrollIndex], texturePackList[this->scrollIndex - 1]);
                        currentPack = texturePackList[this->scrollIndex--];
                        rebuildTextureCache();
                    }
                    break;
                case ItemButtons::DOWN:
                    if(this->scrollIndex < texturePackList.size() - 1){
                        std::swap(texturePackList[this->scrollIndex], texturePackList[this->scrollIndex + 1]);
                        currentPack = texturePackList[this->scrollIndex++];
                        rebuildTextureCache();
                    }
                    break;
                case ItemButtons::TOGGLE:
                    break;
            }
            return;
        }
        currentPack = texturePackList[this->scrollIndex];
        currentSubItem = 0;
    }
    if(IsBtnPressed(MoonButtons::B_BTN)) {
        if(currentPack != NULL){
            currentPack = NULL;
            return;
        }
        MoonChangeUI(0);
    }
    MoonScreen::Update();
}

char *strdup(const char *src_str) noexcept {
    char *new_str = new char[std::strlen(src_str) + 1];
    std::strcpy(new_str, src_str);
    return new_str;
}

void MoonAddonsScreen::Draw(){
    string curTitle = "Texture packs";

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

    int i = 0;
    for(auto &addon : texturePackList){
        bool selected = i == this->scrollIndex && currentPack != NULL;
        int itemWidth = boxWidth - (selected ? 15 : 0);

        MoonDrawRectangle(35, 45 + (i * 35), itemWidth - 20, 31, i == this->scrollIndex && !selected ? focusColor : (Color){0, 0, 0, 100}, true);
        string iconPath = "mod-icons://"+addon->name;
        MoonDrawTexture(35, 45 + (i * 35), 30, 30, strdup(iconPath.c_str()));
        MoonDrawText(70, 45 + (i * 35) + 3, addon->name, 0.8, {255, 255, 255, 255}, true, true);
        MoonDrawText(70, 45 + (i * 35) + 16, addon->description, 0.8, {255, 255, 255, 255}, true, true);

        string rawVer = to_string(addon->version);
        string version = "v"+rawVer.substr(0, rawVer.find(".")+2);

        MoonDrawText(itemWidth + 13 - MoonGetTextWidth(version, 0.5, false),       45 + (i * 35) + 2,      version,       0.5, {255, 255, 255, 255}, true, true);
        MoonDrawText(itemWidth + 13 - MoonGetTextWidth(addon->author, 0.5, false), 45 + (i * 35) + 22, addon->author, 0.5, {255, 255, 255, 255}, true, true);

        if(selected){
            MoonDrawRectangle(itemWidth + 16, 45 + (i * 35),        13, 9.3, currentSubItem == ItemButtons::UP ? focusColor : (Color){0, 0, 0, 100}, true);
            MoonDrawTexture  (itemWidth + 18, 45 + (i * 35),        8, 8, "textures/special/up.rgba16");

            MoonDrawRectangle(itemWidth + 16, 45 + (i * 35) + 10.9, 13, 9.3, currentSubItem == ItemButtons::DOWN ? focusColor : (Color){0, 0, 0, 100}, true);
            MoonDrawTexture  (itemWidth + 18, 46 + (i * 35) + 10.9, 8, 8, "textures/special/down.rgba16");

            MoonDrawRectangle(itemWidth + 16, 45 + (i * 35) + 21.7, 13, 9.3, currentSubItem == ItemButtons::TOGGLE ? focusColor : (Color){0, 0, 0, 100}, true);
            MoonDrawTexture  (itemWidth + 18, 46 + (i * 35) + 21.7, 8, 8, "textures/special/remove.rgba16");
        }
        i++;
    }

    MoonScreen::Draw();
}