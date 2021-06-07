#ifndef MoonScreenAchievements
#define MoonScreenAchievements
#include "moon/ui/interfaces/moon-screen.h"

class MoonAchievementsScreen : public MoonScreen {
public:
    void Init();
    void Update();
    void Draw();
    void Mount();
private:
    void changeScroll(int idx);
    int scrollModifier = 0;

    int focusFlag;
    int focusRange = 80;
    float focusAnim = focusRange / 2;
    bool dispatched;
    bool stickAnim = 0;
};


#endif