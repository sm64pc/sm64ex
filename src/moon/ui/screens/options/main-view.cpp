#include "main-view.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"
#include "moon/ui/screens/options/categories/mcategory.h"

#include "moon/ui/screens/options/categories/mgame.h"
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

void drawButton(int x, int y, string text, string texture, int size, int offset, bool rtl){
    if(!rtl){
        MoonDrawTexture(GFX_DIMENSIONS_FROM_LEFT_EDGE(x), y - 3 + offset, size, size, sys_strdup(texture.c_str()));
        MoonDrawText(x + 16 + 3, y, text, 0.8, {255, 255, 255, 255}, true, false);
    } else {
        x = GetScreenWidth(false) - x;
        int txtWidth = MoonGetTextWidth(text, 0.8, false);

        MoonDrawTexture(GFX_DIMENSIONS_FROM_LEFT_EDGE(x) - txtWidth - size - 3, y - 3 + offset, size, size, sys_strdup(texture.c_str()));
        MoonDrawText(x - txtWidth, y, text, 0.8, {255, 255, 255, 255}, true, false);
    }
}

void MoonOptMain::Draw(){
    string curTitle = categories[categoryIndex]->titleKey ? Moon::getLanguageKey(categories[categoryIndex]->categoryName) : categories[categoryIndex]->categoryName;

    float txtWidth = MoonGetTextWidth(curTitle, 1.0, true);
    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 20, curTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 50, SCREEN_WIDTH - 50, GetScreenHeight() * 0.6, {0, 0, 0, 100}, true);

    string stickTexture = "textures/moon/";

    if(this->selected == NULL){
        stickTexture.append(stickAnim ? "stick-down.rgba16" : "stick-up.rgba16");
        drawButton(5, GetScreenHeight() - 24, "Move", stickTexture, 16, 0, false);
    } else{
        stickTexture.append(stickAnim ? "stick-left.rgba16" : "stick-right.rgba16");
        drawButton(5, GetScreenHeight() - 24, "Change value", stickTexture, 16, 0, false);
    }

    drawButton(7, GetScreenHeight() - 24, this->selected == NULL ? "Select" : "Back", this->selected == NULL ? "textures/moon/a-alt-btn.rgba16" : "textures/moon/b-alt-btn.rgba16", 10, 4, true);

    MoonScreen::Draw();
}

void MoonOptMain::Dispose(){
    configfile_save(configfile_name());
    play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
}