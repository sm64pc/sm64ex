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
    
    bool isFloat = this->bind.fvar != NULL;
    bool isInt   = this->bind.ivar != NULL;

    MoonDrawRectangle(this->x + 10, this->y, barWidth, 16, focusColors[this->selected ? 0 : this->focused ? 1 : 2], true);

    if(this->bind.bvar != NULL){
        bool status = *this->bind.bvar;
        Color toggleColors[] = {
            {32, 255, 3, 255},
            {255, 32, 3, 255}            
        };
        string statusText = status ? Moon_GetKey("TEXT_OPT_ENABLED") : Moon_GetKey("TEXT_OPT_DISABLED");

        tmpWidth += MoonGetTextWidth(statusText, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, statusText, scale, toggleColors[status ? 0 : 1] , true, true);
    } else if(this->bind.values != NULL && this->bind.index != NULL){
        int index = *this->bind.index;
        
        string text = (*this->bind.values)[index];

        tmpWidth += MoonGetTextWidth(text, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, text, scale, {100, 100, 255, 255}, true, true);
    } else if(isFloat || isInt){
        float value = isFloat ? *this->bind.fvar : *this->bind.ivar;
        float max   = this->bind.max;
        
        string text = to_string((int)(100 * (value / max))) + " %";

        tmpWidth += MoonGetTextWidth(text, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, text, scale, {100, 100, 255, 255}, true, true);
    }
    
    MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2, this->y, this->title, scale, {255, 255, 255, 255}, true, true);
}

void MWValue::Update(){
    if(!this->focused) return;

    float xStick = GetStickValue(MoonButtons::L_STICK, false);
    bool isBool  = this->bind.bvar != NULL;
    bool isArray = this->bind.values != NULL && this->bind.index != NULL;

    bool isFloat = this->bind.fvar != NULL;
    bool isInt   = this->bind.ivar != NULL;

    float maxValue = isArray ? (*this->bind.values).size() - 1 : this->bind.max;
    float minValue = isArray ? 0 : this->bind.min;
    float step     = isArray ? 1 : this->bind.step;

    if(xStick < 0) {
        if(mwvStickExecuted) return;
        if(isBool) {
            *this->bind.bvar = !*this->bind.bvar;
            std::cout << "Executed" << std::endl;
        } else if(isArray || isFloat || isInt) {
            float cIndex = isArray ? (int) *this->bind.index : isFloat ? *this->bind.fvar : *this->bind.ivar;
            cout << "Test" << endl;
            if(cIndex > minValue){
                if(isArray) *this->bind.index -= (int)step;
                if(isFloat) *this->bind.fvar  -= step; 
                if(isInt)   *this->bind.ivar  -= (int)step;
            } else {
                if(isArray) *this->bind.index = (int)maxValue;
                if(isFloat) *this->bind.fvar  = maxValue; 
                if(isInt)   *this->bind.ivar  = (int)maxValue;
            }                
            std::cout << "Executed x2" << std::endl;
        }
        mwvStickExecuted = true;
    } 
    if(xStick > 0) {
        if(mwvStickExecuted) return;
        if(isBool) {
            *this->bind.bvar = !*this->bind.bvar;
        } else if(isArray || isFloat || isInt) {
            float cIndex = isArray ? (int) *this->bind.index : isFloat ? *this->bind.fvar : *this->bind.ivar;

            if(cIndex < maxValue){
                if(isArray) *this->bind.index += (int)step;
                if(isFloat) *this->bind.fvar  += step; 
                if(isInt)   *this->bind.ivar  += (int)step;
            } else {
                if(isArray) *this->bind.index = (int)minValue;
                if(isFloat) *this->bind.fvar  = minValue; 
                if(isInt)   *this->bind.ivar  = (int)minValue;
            }
            std::cout << "Executed x3" << std::endl;
        }
        mwvStickExecuted = true;
    }
    if(!xStick)
        mwvStickExecuted = false;
}
void MWValue::Dispose(){}