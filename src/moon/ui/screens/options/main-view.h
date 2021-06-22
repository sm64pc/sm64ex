#ifndef MoonScreenTest
#define MoonScreenTest

#ifdef __cplusplus
#include "moon/ui/interfaces/moon-screen.h"
class MoonOptMain : public MoonScreen {
public:
    void Init();
    void Update();
    void Draw();
    void Mount();
    void Dispose();
private:
    void setCategory(int index);
    bool stickAnim = 0;
};
#else

void drawIngameMenuButtons();

#endif

#endif