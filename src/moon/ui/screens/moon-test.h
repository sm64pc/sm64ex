#ifndef MoonScreenTest
#define MoonScreenTest
#include "moon/ui/interfaces/moon-screen.h"

class MoonTest : public MoonScreen {
public:
    void Init();
    void Draw();
    void Mount();
};


#endif