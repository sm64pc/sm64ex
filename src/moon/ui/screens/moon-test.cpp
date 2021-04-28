#include "moon-test.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/network/moon-consumer.h"

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
}

int x = 0;
int y = 30;

void MoonTest::Draw(){

    if(this->IsDown(MoonButtons::L_CBTN)){
        x -= 1;
    }

    if(this->IsDown(MoonButtons::R_CBTN)){
        x += 1;
    }
    
    if(this->IsDown(MoonButtons::U_CBTN)){
        y -= 1;
    }

    if(this->IsDown(MoonButtons::D_CBTN)){
        y += 1;
    }

    MoonDrawRectangle(0, 0, this->screenWidth, this->screenHeight, {0, 0, 0, 100}, false);
    MoonDrawText(x, y, "This is a test uwu", 1.0, {255, 255, 255, 255}, false);
    MoonDrawColoredText(0, 50, "This is a test owo", 1.0, {255, 255, 255, 255}, false);     
}