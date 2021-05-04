#ifndef MouseIOModule
#define MouseIOModule

#include "moon/io/io-module.h"

enum MouseBtn {
    LEFT_BTN,
    RIGHT_BTN
};

class MouseIO : public MIOModule {
public:
    int xPos;
    int yPos;
    int xLocalPos;
    int yLocalPos;
    int xGlobalPos;
    int yGlobalPos;
    float vScroll;
    float hScroll;
    bool relativeMode;
    bool hideCursor;
    void* window;
    void init();
    void update();
    bool isBtnPressed(MouseBtn btn);
};

#endif