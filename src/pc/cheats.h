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
    bool         SPD;
    bool         TPF;
    unsigned int JB;
    bool         JBC;
    bool         QuikEnd;
    unsigned int Hurt;
    bool         Cann;
    bool         AutoWK;
    bool         GetShell;
    bool         GetBob;
    unsigned int Spamba;
    bool         Swim;
    bool         WingCap;
    bool         MetalCap;
    bool         VanishCap;
    bool         RemoveCap;
    bool         DCM;
    bool         NormalCap;
    unsigned int BLJAnywhere;
};

extern struct CheatList Cheats;

#endif // _CHEATS_H
