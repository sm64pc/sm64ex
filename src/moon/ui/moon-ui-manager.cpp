#include <vector>
#include <iostream>
#include "interfaces/moon-screen.h"
#include "screens/moon-test.h"

extern "C" {
#include "game/game_init.h"
}

using namespace std;

bool isRunning = false;
bool isOpen = false;

int currentScreen = 0;
vector<MoonScreen*> screens;
MoonButtons toggle = MoonButtons::R_BTN;

void MoonUpdateStatus();

void MoonInitUI() {

    if(screens.empty())
        screens.push_back(new MoonTest());

    screens[currentScreen]->Mount();
    screens[currentScreen]->Init();    
}

void MoonDrawUI() {
    MoonUpdateStatus();
    screens[currentScreen]->Update();
    screens[currentScreen]->Draw();
}

void MoonDisposeUI() {
    isRunning = false;
    screens[currentScreen]->Dispose();
}

void MoonChangeUI(int index){
    if(index == -1){
        isOpen = false;        
        return;
    }
    if(!(isOpen && isRunning)) return;
    currentScreen = index;
    screens[currentScreen]->Mount();
    screens[currentScreen]->Init();    
}

void MoonHandleToggle(){
    if(gPlayer1Controller->buttonPressed & toggle){
        isOpen = !isOpen;
        if(isOpen) isRunning = false;
    }
}

void MoonUpdateStatus() {
    if(!isOpen) {
        if(isRunning) MoonDisposeUI();
        return;
    }    
    if(!isRunning) {
        MoonInitUI();
        isRunning = true;
    }
}