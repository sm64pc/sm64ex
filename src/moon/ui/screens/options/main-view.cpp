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
using namespace std;

extern "C" {
#include "sm64.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"
}

vector<MoonCategory*> categories;
bool cswStickExecuted;
int categoryIndex = 0;
string curTitle;

void MoonOptMain::setCategory(int index){
    this->widgets.clear();
    MoonCategory *cat = categories[index];
    vector<MoonWidget*> tmp = cat->catOptions;
    copy(tmp.begin(), tmp.end(), back_inserter(this->widgets));
    curTitle = Moon_GetKey(cat->categoryName);
    MoonScreen::Mount();
}

void MoonOptMain::Mount(){
    this->widgets.clear();
    categories.push_back(new MGameCategory());
    categories.push_back(new MVideoCategory());
    this->setCategory(categoryIndex);
}

void MoonOptMain::Update(){
    if(this->selected->focused){
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
            if(categoryIndex < categories.size())
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
    float txtWidth = MoonGetTextWidth(curTitle, 1.0, true);
    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 20, curTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 50, SCREEN_WIDTH - 50, GetScreenHeight() * 0.6, {0, 0, 0, 100}, true);
    MoonScreen::Draw();
}