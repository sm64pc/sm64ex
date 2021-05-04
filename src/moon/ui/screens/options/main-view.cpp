#include "main-view.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/network/moon-consumer.h"
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

using namespace std;

extern "C" {
#include "sm64.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"
}

vector<MoonCategory*> categories;
bool cswStickExecuted;
int categoryIndex = 0;

void MoonOptMain::setCategory(int index){
    MoonCategory *cat = categories[index];
    this->widgets = cat->catOptions;
    MoonScreen::Mount();
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

void MoonOptMain::Update(){
    if(this->selected == NULL) {
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
    string curTitle = Moon_GetKey(categories[categoryIndex]->categoryName);
    float txtWidth = MoonGetTextWidth(curTitle, 1.0, true);
    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 20, curTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 50, SCREEN_WIDTH - 50, GetScreenHeight() * 0.6, {0, 0, 0, 100}, true);
    MoonScreen::Draw();
}