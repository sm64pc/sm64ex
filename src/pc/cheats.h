#ifndef _CHEATS_H
#define _CHEATS_H

#include <stdbool.h>

struct CheatList {
    bool         EnableCheats;
    bool         MoonJump;
    bool         GodMode;
    bool         InfiniteLives;
    bool         SuperSpeed;
    bool         Responsive;
    bool         ExitAnywhere;
    bool         HugeMario;
    bool         TinyMario;
    bool         WingCap;
    bool         MetalCap;
    bool         VanishCap;
    bool         NormalCap;
};

extern struct CheatList Cheats;

#endif // _CHEATS_H
