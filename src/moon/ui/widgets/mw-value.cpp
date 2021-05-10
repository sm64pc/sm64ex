#include "mw-value.h"
#include <iostream>
#include <string>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/interfaces/moon-screen.h"

using namespace std;

bool mwvStickExecuted;

MWValue::MWValue(float x, float y, std::string title, MWValueBind bind){
    this->x = x;
    this->y = y;
    this->bind = bind;
    this->title = title;
    this->titleKey = false;
}

MWValue::MWValue(float x, float y, std::string title, MWValueBind bind, bool titleKey){
    this->x = x;
    this->y = y;
    this->bind = bind;
    this->title = title;
    this->titleKey = titleKey;
}

int frameCounter = 0;
int focusAnimRange = 80;
float focusAnimation = focusAnimRange / 2;
int focusAnimationPingPong;

void MWValue::Init(){
    mwvStickExecuted = false;
}

void MWValue::Draw(){

    float step = 0.6;

    if(focusAnimation >= focusAnimRange)
        focusAnimationPingPong = 1;
    else if (focusAnimation <= focusAnimRange / 2)
        focusAnimationPingPong = 0;

    focusAnimation += step * (focusAnimationPingPong ? -1 : 1);

    string rawTitle = this->titleKey ? Moon::getLanguageKey(this->title) : this->title;
    float scale = 1;
    float titleWidth = MoonGetTextWidth(rawTitle + " ", scale, false);
    int barWidth = SCREEN_WIDTH - 50 - 14;
    float tmpWidth = titleWidth;

    Color focusColors[] = {
        {255, 255, 255, 40 + focusAnimation / 2},
        {255, 247, 0,   40 + focusAnimation / 2},
        {0, 0, 0, 0},
    };

    bool isFloat = this->bind.fvar != NULL;
    bool isInt   = this->bind.ivar != NULL;

    MoonDrawRectangle(this->x + 10, this->y, barWidth, 16, focusColors[this->selected ? 0 : this->focused ? 1 : 2], true);

    if(this->bind.bvar != NULL){
        bool status = *this->bind.bvar;
        Color toggleColors[] = {
            {61, 255, 113, 255},
            {255, 61, 61, 255}
        };
        string statusText = status ? Moon::getLanguageKey("TEXT_OPT_ENABLED") : Moon::getLanguageKey("TEXT_OPT_DISABLED");

        tmpWidth += MoonGetTextWidth(statusText, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, statusText, scale, toggleColors[status ? 0 : 1] , true, true);
    } else if(this->bind.values != NULL && this->bind.index != NULL){
        int index = *this->bind.index;

        string text = this->bind.valueKeys ? Moon::getLanguageKey((*this->bind.values)[index]) : (*this->bind.values)[index];

        tmpWidth += MoonGetTextWidth(text, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, text, scale, {58, 249, 252, 255}, true, true);
    } else if(isFloat || isInt){
        float value = isFloat ? *this->bind.fvar : *this->bind.ivar;
        float max   = this->bind.max;

        string text = to_string((int)(100 * (value / max))) + "%";

        tmpWidth += MoonGetTextWidth(text, scale, false);
        MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, text, scale, {58, 249, 252, 255}, true, true);
    }

    if(this->bind.btn != NULL)
        tmpWidth = titleWidth;

    MoonDrawText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2, this->y, rawTitle, scale, {255, 255, 255, 255}, true, true);
}

void MWValue::Update(){
    if(!this->focused) return;

    float xStick = GetStickValue(MoonButtons::L_STICK, false);
    bool isBool  = this->bind.bvar   != NULL;
    bool isArray = this->bind.values != NULL && this->bind.index != NULL;
    bool isFloat = this->bind.fvar   != NULL;
    bool isInt   = this->bind.ivar   != NULL;
    bool isBtn   = this->bind.btn    != NULL;

    float maxValue = isArray ? (*this->bind.values).size() - 1 : this->bind.max;
    float minValue = isArray ? 0 : this->bind.min;
    float step     = isArray ? 1 : this->bind.step;

    if(isBtn && IsBtnPressed(MoonButtons::A_BTN)){
        this->bind.btn();
        return;
    }

    if(xStick < 0) {
        if(mwvStickExecuted) {
            if(isBtn || isBool) return;
            if(frameCounter <= 20){
                frameCounter++;
                return;
            }
        }
        if(isBool) {
            *this->bind.bvar = !*this->bind.bvar;
        } else if(isArray || isFloat || isInt) {
            float cIndex = isArray ? (int) *this->bind.index : isFloat ? *this->bind.fvar : *this->bind.ivar;
            if(cIndex > minValue){
                if(isArray) *this->bind.index -= (int)step;
                if(isFloat) *this->bind.fvar  -= step;
                if(isInt)   *this->bind.ivar  -= (int)step;
            } else {
                if(isArray) *this->bind.index = (int)maxValue;
                if(isFloat) *this->bind.fvar  = maxValue;
                if(isInt)   *this->bind.ivar  = (int)maxValue;
            }
        }
        if(this->bind.callback != NULL) this->bind.callback();
        mwvStickExecuted = true;
    }
    if(xStick > 0) {
        if(mwvStickExecuted) {
            if(isBtn || isBool) return;
            if(frameCounter <= 20){
                frameCounter++;
                return;
            }
        }
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
        }
        if(this->bind.callback != NULL) this->bind.callback();
        mwvStickExecuted = true;
    }
    if(!xStick){
        mwvStickExecuted = false;
        frameCounter = 0;
    }
}
void MWValue::Dispose(){}