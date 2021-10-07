#include <vector>
#include <iostream>
#include "interfaces/moon-screen.h"
#include "screens/options/main-view.h"
#include "screens/addons/addons-view.h"
#include "screens/achievements/achievements-view.h"

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

    if(screens.empty()){
        screens.push_back(new MoonOptMain());
        screens.push_back(new MoonAddonsScreen());
        screens.push_back(new MoonAchievementsScreen());
    }

    screens[currentScreen]->Mount();
    screens[currentScreen]->Init();
}

void MoonDrawUI() {
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
    /*
    if(gPlayer1Controller->buttonPressed & toggle){
        currentScreen = 0;
        isOpen = !isOpen;
        MoonUpdateStatus();
    }
    */
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