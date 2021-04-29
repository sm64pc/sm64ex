#include "moon-test.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/network/moon-consumer.h"
#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "sm64.h"
#include "gfx_dimensions.h"
}

void MoonTest::Init(){    
    cout << "Screen called" << endl;    
    //MoonConsumer consumer;
    //consumer.Init();
    //MoonResponse res;
    //MoonRequest req;
    //req.url = "https://raw.githubusercontent.com/Render96/Render96ex_Languages/master/PT_br.json";
    // req.file = "Kalimba2.txt";
    //consumer.Get(req, &res);
    //printf("%s\n", res.body.c_str());
    MoonScreen::Init();
}
bool b = true;
bool c = true;
bool d = true;
float  e = 0;

int bIndex = 0;
vector<string> test = {"Val zero", "Val uwo", "Val owu"};

void MoonTest::Mount(){
    this->widgets.clear();
    this->widgets.push_back(new MWValue({.index = &bIndex, .values = &test}, "Toggle 3 owo", 25, 95));
    this->widgets.push_back(new MWValue({.bvar = &b}, "Toggle owo", 25, 55));
    this->widgets.push_back(new MWValue({.fvar = &e, .max = 10, .min = 0, .step = 0.1f}, "Toggle 2 owo", 25, 75));    
    MoonScreen::Mount();
}

int x = 0;
int y = 20;

void MoonTest::Draw(){
    MoonDrawText(0, 0, "Test text", 1.0, {255, 255, 255, 255}, true, false);

    string menuTitle = "Placeholder";
    float txtWidth = MoonGetTextWidth(menuTitle, 1.0, true);
    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 20, menuTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 50, SCREEN_WIDTH - 50, GetScreenHeight() * 0.6, {0, 0, 0, 100}, true);
    MoonScreen::Draw();
}