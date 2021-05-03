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

bool  b = true;
float e = 0;
int bIndex = 0;

int rIndex = 0;

vector<string> test = {"Val zero", "Val uwo", "Val owu"};
vector<string> randomize = {"owo", "awa", "uwu", "wololo", "idk"};

MWValue * testBtn;

void testF(){
    rIndex = rand() % randomize.size();
    testBtn->title = randomize[rIndex];
    cout << rIndex << endl;
}

void MoonTest::Mount(){
    this->widgets.clear();    
    this->widgets.push_back(new MWValue(22, 57,  "Bool:",   {.bvar = &b}));
    this->widgets.push_back(new MWValue(22, 74,  "Number:", {.fvar = &e, .max = 10, .min = 0, .step = 0.1f}));    
    this->widgets.push_back(new MWValue(22, 91, "Array:", {.index = &bIndex, .values = &test}));
    this->widgets.push_back(testBtn = new MWValue(22, 108, "Randomize", {.btn = testF}));
    MoonScreen::Mount();
}

int x = 0;
int y = 20;

void MoonTest::Draw(){
    MoonDrawText(0, 0, "Hi uwu", 0.5, {255, 255, 255, 255}, true, false);

    string menuTitle = "Placeholder";
    float txtWidth = MoonGetTextWidth(menuTitle, 1.0, true);
    MoonDrawRectangle(0, 0, GetScreenWidth(false), GetScreenHeight(), {0, 0, 0, 100}, false);
    MoonDrawColoredText(SCREEN_WIDTH / 2 - txtWidth / 2, 20, menuTitle, 1.0, {255, 255, 255, 255}, true, true);
    MoonDrawRectangle(25, 50, SCREEN_WIDTH - 50, GetScreenHeight() * 0.6, {0, 0, 0, 100}, true);
    MoonScreen::Draw();
}