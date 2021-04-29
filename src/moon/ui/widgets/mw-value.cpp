#include "mw-value.h"
#include <iostream>
#include <string>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/interfaces/moon-screen.h"

using namespace std;

bool mwvStickExecuted;

MWValue::MWValue(MWValueBind bind, std::string title, float x, float y){
    this->x = x;
    this->y = y;
    this->bind = bind;
    this->title = title;
}

void MWValue::Init(){
    mwvStickExecuted = false;
}
void MWValue::Draw(){
    float scale = 1;
    float titleWidth = MoonGetTextWidth(this->title + " ", scale, false);
    int barWidth = SCREEN_WIDTH - 50 - 20;
    float tmpWidth = titleWidth;
    

    Color focusColors[] = { 
        {255, 255, 255, 80},
        {255, 247, 0, 80},
        {0, 0, 0, 0},
    };
    
    MoonDrawRectangle(this->x + 10, this->y, barWidth, 16, focusColors[this->selected ? 0 : this->focused ? 1 : 2], true);

    if(this->bind.bvar != NULL){
        bool status = *this->bind.bvar;
        Color toggleColors[] = { 
            {255, 32, 3, 255},
            {32, 255, 32, 255}
        };
        string statusText = status ? Moon_GetKey("TEXT_OPT_ENABLED") : Moon_GetKey("TEXT_OPT_DISABLED");

        tmpWidth += MoonGetTextWidth(statusText, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, statusText, scale, toggleColors[status ? 0 : 1] , true, false);        
    }

    MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2, this->y, this->title, scale, {255, 255, 255, 255}, true, false);
}

void MWValue::Update(){
    float xStick = GetStickValue(MoonButtons::L_STICK, false);

    if(xStick > 0) {
        if(mwvStickExecuted) return;
        if(this->bind.bvar != NULL && this->focused ) {
            *this->bind.bvar = !*this->bind.bvar;
            std::cout << "Executed" << std::endl;
        }
        mwvStickExecuted = true;
    } 
    if(xStick < 0) {
        if(mwvStickExecuted) return;
        if(this->bind.bvar != NULL && this->focused ) {
            *this->bind.bvar = !*this->bind.bvar;
            std::cout << "Executed" << std::endl;
        }            
        mwvStickExecuted = true;
    }
    if(!xStick)
        mwvStickExecuted = false;
}
void MWValue::Dispose(){}