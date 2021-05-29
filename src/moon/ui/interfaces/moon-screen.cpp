#include "moon-screen.h"
#include <algorithm>
#include <iostream>

extern "C" {
#include "engine/math_util.h"
#include "sm64.h"
#include "game/game_init.h"
#include "gfx_dimensions.h"
#include "game/mario_misc.h"
#include "audio/external.h"
}

int scrollWidgetModifier = 0;

void MoonScreen::Init(){
    this->scrollIndex = 0;
    this->selected = NULL;

    if(this->enabledWidgets)
        for(int i = 0; i < widgets.size(); i++){
            widgets[i]->Init();
            widgets[i]->parent = this;
        }
}

void MoonScreen::Mount(){
    if(!this->widgets.empty()){
        this->scrollIndex = 0;
        this->selected  = NULL;
        for(int i = 0; i < widgets.size(); i++) {
            widgets[i]->selected = false;
            widgets[i]->focused = false;
            widgets[i]->parent = this;
        }
        MoonWidget* sel = this->widgets[this->scrollIndex];
        sel->selected = true;
        sel->focused = false;
    }
}

void MoonScreen::Draw(){
    if(this->enabledWidgets){
        for(int i = 0; i < widgets.size(); i++)
            widgets[i]->Draw();
        //int widgetAmount = widgets.size();
        //int maxWidgets = 8;
        //int iMod = scrollWidgetModifier;

        //for(int i = 0; i < min(widgetAmount, maxWidgets); i++){
        //    int index = i + iMod;

        //    if(index > widgetAmount - 1){
        //        this->scrollIndex = 0;
        //        scrollWidgetModifier = 0;
        //        return;
        //    }

        //    widgets[index]->parent = this;
        //    widgets[index]->Draw();
        //    widgets[index]->mY = 15 * iMod;
        //}
    }
}

bool stickExecuted;


void MoonScreen::changeScroll(int idx){
    if(idx < 0){
        if(this->scrollIndex > 0){
            if(scrollWidgetModifier > 0 && this->scrollIndex == scrollWidgetModifier)
                scrollWidgetModifier--;
            this->scrollIndex--;
            return;
        }
        this->scrollIndex = this->widgets.size() - 1;
        scrollWidgetModifier = this->scrollIndex - 4;
        return;
    }
    if(this->scrollIndex < this->widgets.size() - 1){
        if(this->scrollIndex > 3 && !((this->scrollIndex - scrollWidgetModifier) % 4)) scrollWidgetModifier++;
        this->scrollIndex++;
        return;
    }

    this->scrollIndex = 0;
    scrollWidgetModifier = 0;
}

void MoonScreen::Update(){
    if(this->enabledWidgets && !this->widgets.empty()) {
        if(this->useMouseInstead){
            // MouseIO* tmp = GetIOModule<MouseIO>();
            // if(tmp != NULL){
            //     int mouseX = tmp->xLocalPos;
            //     int mouseY = tmp->yLocalPos;
            //     int baseY = 119;
            //     int optionSize = 16;
            //     if(mouseX >= 22 && mouseY > baseY){
            //         int currentId = ((mouseY - baseY) / (optionSize * 2));
            //         if(currentId < widgets.size()){
            //             for(int i = 0; i < widgets.size(); i++) {
            //                 widgets[i]->selected = false;
            //                 widgets[i]->focused = false;
            //             }
            //             this->widgets[currentId]->selected = false;
            //             this->widgets[currentId]->focused = true;
            //         }
            //     }
            // }
            // return;
        }
        float yStick = GetStickValue(MoonButtons::U_STICK, false);
        if(yStick > 0) {
            if(stickExecuted) return;
            if(this->selected != NULL) return;
            this->widgets[this->scrollIndex]->selected = false;

            MoonScreen::changeScroll(-1);

            this->widgets[this->scrollIndex]->selected = true;
            stickExecuted = true;
        }
        if(yStick < 0) {
            if(stickExecuted) return;
            if(this->selected != NULL) return;
            this->widgets[this->scrollIndex]->selected = false;

            MoonScreen::changeScroll(1);

            this->widgets[this->scrollIndex]->selected = true;
            stickExecuted = true;
        }
        if(!yStick)
            stickExecuted = false;
        if(IsBtnPressed(MoonButtons::A_BTN)) {
            this->selected = this->widgets[this->scrollIndex];
            this->selected->selected = false;
            this->selected->focused = true;
        }
        if(IsBtnPressed(MoonButtons::B_BTN)) {
            if(this->selected != NULL){
                this->selected->selected = true;
                this->selected->focused = false;
                this->selected = NULL;
            }
        }
        for(int i = 0; i < widgets.size(); i++){
            widgets[i]->parent = this;
            widgets[i]->Update();
        }
    }
}

void MoonScreen::Dispose(){
    if(this->enabledWidgets)
        for(int i = 0; i < widgets.size(); i++)
            widgets[i]->Dispose();
}

bool IsBtnPressed(MoonButtons button){
    return gPlayer1Controller->buttonPressed & button;
}

bool IsBtnDown(MoonButtons button){
    return gPlayer1Controller->buttonDown & button;
}

float GetStickValue(MoonButtons button, bool absolute){
    switch(button){
        case MoonButtons::L_STICK:
        case MoonButtons::R_STICK:
            return absolute ? abs(gPlayer1Controller->stickX) : gPlayer1Controller->stickX;
        case MoonButtons::U_STICK:
        case MoonButtons::D_STICK:
            return absolute ? abs(gPlayer1Controller->stickY) : gPlayer1Controller->stickY;
        default:
            return 0.f;
    }
}

float GetScreenWidth(bool u4_3){
    int brds = GFX_DIMENSIONS_FROM_LEFT_EDGE(0);
    return ceil(brds < 0 && !u4_3 ? (SCREEN_WIDTH + abs(brds) * 2) + 2 : SCREEN_WIDTH - abs(brds) * 2);
}

float GetScreenHeight() {
    return SCREEN_HEIGHT;
}