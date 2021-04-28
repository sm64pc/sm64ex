#include "moon-screen.h"

extern "C" {
#include "sm64.h"
#include "game/game_init.h"
#include "gfx_dimensions.h"
#include "game/mario_misc.h"
}

void MoonScreen::Init(){
    for(int i = 0; i < widgets.size(); i++)
        widgets[i]->Init();
}

void MoonScreen::Mount(){}

void MoonScreen::Draw(){
    this->screenWidth  = SCREEN_WIDTH + abs(GFX_DIMENSIONS_FROM_LEFT_EDGE(0)) * 2;
    this->screenHeight = SCREEN_HEIGHT;
    for(int i = 0; i < widgets.size(); i++)
        widgets[i]->Draw();   
}

void MoonScreen::Update(){
    for(int i = 0; i < widgets.size(); i++)
        widgets[i]->Update();
}

void MoonScreen::Dispose(){
    for(int i = 0; i < widgets.size(); i++)
        widgets[i]->Dispose();
}

bool MoonScreen::IsPressed(MoonButtons button){
    return gPlayer1Controller->buttonPressed & button;
}

bool MoonScreen::IsDown(MoonButtons button){
    return gPlayer1Controller->buttonDown & button;
}

float GetValue(MoonButtons button){
    switch(button){
        case MoonButtons::L_STICK:
        case MoonButtons::R_STICK:
            return gPlayer1Controller->stickX;
        case MoonButtons::U_STICK:
        case MoonButtons::D_STICK:
            return gPlayer1Controller->stickY;
        default:
            return 0.f;
    }
}