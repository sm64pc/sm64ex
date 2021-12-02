#ifndef MoonScreenAddons
#define MoonScreenAddons
#include "moon/ui/interfaces/moon-screen.h"

class MoonAddonsScreen : public MoonScreen {
public:
    void Init();
    void Update();
    void Draw();
    void Mount();
private:
    bool stickAnim = 0;
    void changeScroll(int idx);
};

#include <vector>
#include "moon/mod-engine/interfaces/bit-module.h"

extern BitModule* currentPack;
extern std::vector<BitModule*> texturePackList;

extern void rebuildTextureCache();

#endif