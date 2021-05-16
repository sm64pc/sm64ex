#ifndef MoonScreenTest
#define MoonScreenTest
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
};


#endif