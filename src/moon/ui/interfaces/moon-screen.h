#ifndef MoonScreenInterface
#define MoonScreenInterface

#include "moon-widget.h"
#include <vector>

enum MoonButtons {
    A_BTN   = 0x8000, B_BTN   = 0x4000,
    L_BTN   = 0x0020, R_BTN   = 0x0010,
    Z_BTN   = 0x2000, START   = 0x1000,
    U_STICK = 0x0800, D_STICK = 0x0400,
    L_STICK = 0x0200, R_STICK = 0x0100,
    U_CBTN  = 0x0008, D_CBTN  = 0x0004,
    L_CBTN  = 0x0002, R_CBTN  = 0x0001
};

class MoonScreen {
protected:
    std::vector<MoonWidget*> widgets;
    MoonWidget* selected;
    bool enabledWidgets = true;
    bool useMouseInstead = false; // unused
    int  scrollIndex = 0;
public:
    virtual void Init();
    virtual void Mount();
    virtual void Draw();
    virtual void Update();
    virtual void Dispose();
};

bool IsBtnPressed(MoonButtons button);
bool IsBtnDown(MoonButtons button);
float GetStickValue(MoonButtons button, bool absolute);

float GetScreenWidth(bool u4_3);
float GetScreenHeight();

#endif