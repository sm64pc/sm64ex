#include "moon-screen.h"
#include <algorithm>
#include <iostream>

extern "C" {
#include "engine/math_util.h"
#include "sm64.h"
#include "game/game_init.h"
#include "gfx_dimensions.h"
#include "game/mario_misc.h"
}

void MoonScreen::Init(){
    this->scrollIndex = 0;
    this->selected = NULL;

    if(this->enabledWidgets)
        for(int i = 0; i < widgets.size(); i++)
            widgets[i]->Init();
}

void MoonScreen::Mount(){
    if(!this->widgets.empty()){
        this->selected = this->widgets[0];
        this->selected->selected = true;
        this->selected->focused = false;
    }
}

void MoonScreen::Draw(){
    if(this->enabledWidgets)
        for(int i = 0; i < widgets.size(); i++)
            widgets[i]->Draw();
}

bool stickExecuted;

void MoonScreen::Update(){
    this->screenWidth  = GFX_DIMENSIONS_ASPECT_RATIO > 1 ? SCREEN_WIDTH + abs(GFX_DIMENSIONS_FROM_LEFT_EDGE(0)) * 2 : SCREEN_WIDTH;
    this->screenHeight = SCREEN_HEIGHT;
    if(this->enabledWidgets) {
        
        float xStick = this->GetValue(MoonButtons::L_STICK, false);
        float yStick = this->GetValue(MoonButtons::U_STICK, false);
        
        if(!this->widgets.empty()){
            if(yStick > 0) {
                if(stickExecuted) return;
                if(this->selected != NULL) return;
                this->widgets[this->scrollIndex]->selected = false;
                if(this->scrollIndex > 0)
                    this->scrollIndex--;
                else
                    this->scrollIndex = this->widgets.size() - 1;
                this->widgets[this->scrollIndex]->selected = true;
                stickExecuted = true;
            } 
            if(yStick < 0) {
                if(stickExecuted) return;
                if(this->selected != NULL) return;
                this->widgets[this->scrollIndex]->selected = false;
                if(this->scrollIndex < this->widgets.size() - 1)
                    this->scrollIndex++;
                else
                    this->scrollIndex = 0;
                this->widgets[this->scrollIndex]->selected = true;
                stickExecuted = true;
            }
            if(!yStick)
                stickExecuted = false;
            if(this->IsPressed(MoonButtons::A_BTN)) {
                this->selected = this->widgets[this->scrollIndex];
                this->selected->selected = false;
                this->selected->focused = true;
            } 
            if(this->IsPressed(MoonButtons::B_BTN)) {
                if(this->selected != NULL){
                    this->selected->selected = true;
                    this->selected->focused = false;
                    this->selected = NULL;
                }
            }
        }
        for(int i = 0; i < widgets.size(); i++)
            widgets[i]->Update();
    }
}

void MoonScreen::Dispose(){
    if(this->enabledWidgets)
        for(int i = 0; i < widgets.size(); i++)
            widgets[i]->Dispose();
}

bool MoonScreen::IsPressed(MoonButtons button){
    return gPlayer1Controller->buttonPressed & button;
}

bool MoonScreen::IsDown(MoonButtons button){
    return gPlayer1Controller->buttonDown & button;
}

float MoonScreen::GetValue(MoonButtons button, bool absolute){
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