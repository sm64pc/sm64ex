#include "moon-test.h"
#include <iostream>
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/network/moon-consumer.h"

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

void MoonTest::Mount(){
    this->widgets.clear();
    int a = 25;    
    this->widgets.push_back(new MWValue(&a, 0, 20, MWValueType::INT));
    this->widgets.push_back(new MWValue(&a, 0, 40, MWValueType::INT));
    this->widgets.push_back(new MWValue(&a, 0, 60, MWValueType::INT));
    this->widgets.push_back(new MWValue(&a, 0, 80, MWValueType::INT));

    MoonScreen::Mount();
}

int x = 0;
int y = 30;

void MoonTest::Draw(){

    //if(this->IsDown(MoonButtons::L_CBTN)){
    //    x -= 1;
    //}

    //if(this->IsDown(MoonButtons::R_CBTN)){
    //    x += 1;
    //}
    //
    //if(this->IsDown(MoonButtons::U_CBTN)){
    //    y -= 1;
    //}

    //if(this->IsDown(MoonButtons::D_CBTN)){
    //    y += 1;
    //}

    string test = "Placeholder";
    float txtWidth = MoonGetTextWidth(test, 1.0, false);
    MoonDrawRectangle(0, 0, this->screenWidth, this->screenHeight, {0, 0, 0, 255}, false);    
    MoonDrawText(this->screenWidth / 2 - txtWidth / 2, y, test, 1.0, {255, 255, 255, 255}, false);
    // MoonDrawText(0, 40, test, 1.0, {255, 255, 255, 255}, false);    
    //MoonDrawColoredText(0, 50, "This is a test owo", 1.0, {255, 255, 255, 255}, false);    

    // std::cout << this->screenWidth << " : " << GFX_DIMENSIONS_ASPECT_RATIO << std::endl;

    MoonScreen::Draw();
}