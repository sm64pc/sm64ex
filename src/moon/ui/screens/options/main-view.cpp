#include "main-view.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/ui/screens/options/categories/mcategory.h"

#include "moon/ui/screens/options/categories/mgame.h"
#include "moon/ui/screens/options/categories/mdebug.h"
#include "moon/ui/screens/options/categories/mvideo.h"
#include "moon/ui/screens/options/categories/maudio.h"
#include "moon/ui/screens/options/categories/mcheats.h"
#ifdef BETTERCAMERA
#include "moon/ui/screens/options/categories/mcamera.h"
#endif

#include "moon/io/moon-io.h"
#include "moon/io/modules/mouse-io.h"

using namespace std;

extern "C" {
#include "sm64.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"
#include "audio/external.h"
#include "game/game_init.h"
#include "pc/platform.h"
}

vector<MoonCategory*> categories;
bool cswStickExecuted;
int categoryIndex = 0;

void MoonOptMain::setCategory(int index){
    MoonCategory *cat = categories[index];
    this->widgets = cat->catOptions;
    play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
    MoonScreen::Mount();
}

void MoonOptMain::Init(){
    this->useMouseInstead = true;
    play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
}

void MoonOptMain::Mount(){
    this->widgets.clear();
    categories.push_back(new MGameCategory());
#ifdef GAME_DEBUG
    categories.push_back(new MDebugCategory());
#endif
#ifdef BETTERCAMERA
    categories.push_back(new MCameraCategory());
#endif
    categories.push_back(new MVideoCategory());
    categories.push_back(new MAudioCategory());
    categories.push_back(new MCheatsCategory());
    this->setCategory(categoryIndex);
    MoonScreen::Mount();
}

bool stickAnim = 0;

void MoonOptMain::Update(){

    if(!(gGlobalTimer % 20))
        stickAnim = !stickAnim;

    if(this->selected == NULL) {
        if(IsBtnPressed(MoonButtons::B_BTN)){
            isOpen = false;
        }
        float xStick = GetStickValue(MoonButtons::L_STICK, false);
        if(xStick < 0) {
            if(cswStickExecuted) return;
            if(categoryIndex > 0)
                categoryIndex -= 1;
            else
                categoryIndex = categories.size() - 1;
            this->setCategory(categoryIndex);
            cswStickExecuted = true;
        }
        if(xStick > 0) {
            if(cswStickExecuted) return;
            if(categoryIndex < categories.size() - 1)
                categoryIndex += 1;
            else
                categoryIndex = 0;
            this->setCategory(categoryIndex);
            cswStickExecuted = true;
        }
        if(!xStick)
            cswStickExecuted = false;
    }
    MoonScreen::Update();
}

void MoonOptMain::Draw(){
    wstring curTitle = categories[categoryIndex]->titleKey ? Moon::getLanguageKey(categories[categoryIndex]->categoryName) : categories[categoryIndex]->categoryName;

    float txtWidth = MoonGetTextWidth(curTitle, 1.0, true);
    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawWideColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 20, curTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 50, SCREEN_WIDTH - 50, GetScreenHeight() * 0.6 + 5, {0, 0, 0, 100}, true);

    string basePath = "textures/moon/controller/";

    if(this->selected == NULL){
        basePath.append(stickAnim ? "stick-down.rgba16" : "stick-up.rgba16");
        MoonDrawButton(5, GetScreenHeight() - 24, "Move", basePath, 16, 0, false);
    } else{
        basePath.append(stickAnim ? "stick-left.rgba16" : "stick-right.rgba16");
        MoonDrawButton(5, GetScreenHeight() - 24, "Change value", basePath, 16, 0, false);
    }

    MoonDrawButton(7, pGetScreenHeight() - 24, this->selected == NULL ? "Select" : "Back", this->selected == NULL ? "textures/moon/controller/a-alt-btn.rgba16" : "textures/moon/controller/b-alt-btn.rgba16", 10, 4, true);

    MoonScreen::Draw();
}


void MoonOptMain::Dispose(){
    configfile_save(configfile_name());
    play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
}