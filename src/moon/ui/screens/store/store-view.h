#ifndef MoonScreenStore
#define MoonScreenStore
#include "moon/ui/interfaces/moon-screen.h"

class MoonStoreScreen : public MoonScreen {
public:
    void Init();
    void Update();
    void Draw();
    void Mount();
private:
    void changeScroll(int idx);
};


#endif