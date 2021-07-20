#include "mw-value.h"
#include <iostream>
#include <string>
#include <iomanip>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/interfaces/moon-screen.h"
#include <algorithm>

extern "C" {
#include "pc/platform.h"
#include "pc/configfile.h"
#include "pc/controller/controller_api.h"
}

using namespace std;

bool mwvStickExecuted;
bool focusMode;
bool shouldToggleFocus;
int tmpIdx;

MWValue::MWValue(float x, float y, std::wstring title, MWValueBind bind){
    this->x = x;
    this->bind = bind;
    this->title = title;
    this->titleKey = false;
}

MWValue::MWValue(float x, float y, std::wstring title, MWValueBind bind, bool titleKey){
    this->x = x;
    this->bind = bind;
    this->title = title;
    this->titleKey = titleKey;
}

MWValue::MWValue(float x, float y, std::string title, MWValueBind bind){
    this->x = x;
    this->bind = bind;
    this->title = wide(title);
    this->titleKey = false;
}

MWValue::MWValue(float x, float y, std::string title, MWValueBind bind, bool titleKey){
    this->x = x;
    this->bind = bind;
    this->title = wide(title);
    this->titleKey = titleKey;
}

int frameCounter = 0;
int focusAnimRange = 80;
float focusAnimation = focusAnimRange / 2;
int focusAnimationPingPong;

void MWValue::Init(){
    mwvStickExecuted = false;
    tmpIdx = 0;
    focusMode = false;
}

void MWValue::Draw(){

    float step = 0.6;

    if(focusAnimation >= focusAnimRange)
        focusAnimationPingPong = 1;
    else if (focusAnimation <= focusAnimRange / 2)
        focusAnimationPingPong = 0;

    focusAnimation += step * (focusAnimationPingPong ? -1 : 1);

    wstring rawTitle = this->titleKey ? Moon::getLanguageKey(narrow(this->title)) : this->title;
    float scale = 1;
    float titleWidth = MoonGetTextWidth(rawTitle + L" ", scale, false);
    int barWidth = SCREEN_WIDTH - 50 - 14;
    float tmpWidth = titleWidth;

    Color focusColors[] = {
        {255, 255, 255, 40 + focusAnimation / 2},
        {255, 247, 0,   40 + focusAnimation / 2},
        {0, 0, 0, 0},
    };

    bool isFloat = this->bind.fvar     != NULL;
    bool isInt   = this->bind.ivar     != NULL;
    bool isBind  = this->bind.bindKeys != NULL;

    MoonDrawRectangle(this->x + 10, this->y, barWidth, this->height, focusColors[this->selected ? 0 : this->focused ? 1 : 2], true);

    if(this->bind.bvar != NULL){
        bool status = *this->bind.bvar;
        Color toggleColors[] = {
            {61, 255, 113, 255},
            {255, 61, 61, 255}
        };
        wstring statusText = status ? Moon::getLanguageKey("TEXT_OPT_ENABLED") : Moon::getLanguageKey("TEXT_OPT_DISABLED");

        tmpWidth += MoonGetTextWidth(statusText, scale, false);
        MoonDrawWideText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, statusText, scale, toggleColors[status ? 0 : 1] , true, true);
    } else if(this->bind.values != NULL && this->bind.index != NULL){
        int index = *this->bind.index;

        wstring text = this->bind.valueKeys ? Moon::getLanguageKey((*this->bind.values)[index]) : (*this->bind.values)[index];

        tmpWidth += MoonGetTextWidth(text, scale, false);
        MoonDrawWideText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, text, scale, {58, 249, 252, 255}, true, true);
    } else if(isFloat || isInt){
        float value = isFloat ? *this->bind.fvar : *this->bind.ivar;
        float max   = this->bind.max;

        wstring text = !this->bind.rawValue ? to_wstring((int)(100 * (value / max))) + L"%" : to_wstring((int)value);

        tmpWidth += MoonGetTextWidth(text, scale, false);
        MoonDrawWideText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2 + titleWidth, this->y, text, scale, {58, 249, 252, 255}, true, true);
    } else if(isBind){
        string basePath = "textures/moon/controller/" + this->bind.keyIcon;
        MoonDrawTexture (this->x + 15,      this->y + 1, 13, 13, sys_strdup(basePath.c_str()));
        MoonDrawWideText(this->x + 19 + 15, this->y, rawTitle, 1, {255, 255, 255, 255}, true, true);

        int boxWidth = 0;
        int width = MoonGetTextWidth("NONE", 1.0, false);

        for(int i = 0; i < this->bind.max; i++){
            int key = this->bind.bindKeys[i];
            std::stringstream stream;
            stream << std::setfill ('0') << std::setw(4)  << std::hex << key;
            string tmp = string(stream.str());
            transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
            int aVal = 170 + focusAnimation;

            wstring keyText = key == 0xFFFF || ( this->focused && tmpIdx == i && focusMode) ? L"XXXX" : wide(tmp);
            MoonDrawWideText((SCREEN_WIDTH - 65) - (((width + 5) * ((this->bind.max - 1) - i))), this->y, keyText, 1, this->focused && tmpIdx == i ? focusMode ? (Color) {255, aVal, 0, 255} : (Color) {58, aVal, 252, 255} : (Color) {255, 255, 255, 255}, true, true);
        }
    }

    if(this->bind.btn != NULL)
        tmpWidth = titleWidth;
    if(!isBind)
        MoonDrawWideText(this->x + ( 10 + barWidth / 2 ) - tmpWidth / 2, this->y, rawTitle, scale, {255, 255, 255, 255}, true, true);
}

void unselectWidget(MWValue* base){
    base->focused = false;
    base->selected = true;
    base->parent->selected = nullptr;
}

void MWValue::Update(){
    if(IsBtnPressed(MoonButtons::A_BTN) && this->bind.btn != nullptr && this->selected){
        this->bind.btn();
        return;
    }

    if(!this->focused) return;

    float xStick = GetStickValue(MoonButtons::L_STICK, false);
    bool isBool  = this->bind.bvar     != NULL;
    bool isArray = this->bind.values   != NULL && this->bind.index != NULL;
    bool isFloat = this->bind.fvar     != NULL;
    bool isInt   = this->bind.ivar     != NULL;
    bool isBind  = this->bind.bindKeys != NULL;
    bool isBtn   = this->bind.btn      != NULL;

    float maxValue = isArray ? (*this->bind.values).size() - 1 : this->bind.max - int(isBind);
    float minValue = isArray || isBind ? 0 : this->bind.min;
    float step     = isArray || isBind ? 1 : this->bind.step;

    if(IsBtnPressed(MoonButtons::A_BTN)){
        if(isBtn){
            this->bind.btn();
            return;
        }
        if(isBind && !focusMode){
            focusMode = true;
            mwvStickExecuted = true;
            return;
        }
    }

    if(IsBtnPressed(MoonButtons::B_BTN)){
        if(isBind && !focusMode){
            focusMode = false;
            return;
        }
    }

    if(focusMode){
        u32 key = controller_get_raw_key();
        if (key != VK_INVALID) {
            if(mwvStickExecuted) return;
            this->bind.bindKeys[tmpIdx] = key;
            controller_reconfigure();
            configfile_save(configfile_name());
            focusMode = false;
        } else mwvStickExecuted = false;
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
        } else if(isArray || isFloat || isInt || isBind) {
            float cIndex = isArray ? (int) *this->bind.index : isFloat ? *this->bind.fvar : isBind ? tmpIdx : *this->bind.ivar;
            if(cIndex > minValue){
                if(isArray) *this->bind.index -= (int) step;
                if(isFloat) *this->bind.fvar  -=       step;
                if(isInt)   *this->bind.ivar  -= (int) step;
                if(isBind && !focusMode) tmpIdx -= (int) step;
            } else {
                if(isArray) *this->bind.index = (int) maxValue;
                if(isFloat) *this->bind.fvar  =       maxValue;
                if(isInt)   *this->bind.ivar  = (int) maxValue;
                if(isBind && !focusMode) tmpIdx = (int) maxValue;
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
        } else if(isArray || isFloat || isInt || isBind) {
            float cIndex = isArray ? (int) *this->bind.index : isFloat ? *this->bind.fvar : isBind ? tmpIdx : *this->bind.ivar;

            if(cIndex < maxValue){
                if(isArray) *this->bind.index += (int) step;
                if(isFloat) *this->bind.fvar  +=       step;
                if(isInt)   *this->bind.ivar  += (int) step;
                if(isBind && !focusMode) tmpIdx += (int) step;
            } else {
                if(isArray) *this->bind.index = (int) minValue;
                if(isFloat) *this->bind.fvar  =       minValue;
                if(isInt)   *this->bind.ivar  = (int) minValue;
                if(isBind && !focusMode) tmpIdx = (int) minValue;
            }
        }
        if(this->bind.callback != NULL) this->bind.callback();
        mwvStickExecuted = true;
    }
    if(!xStick && !focusMode){
        mwvStickExecuted = false;
        frameCounter = 0;
    }
}
void MWValue::Dispose(){}