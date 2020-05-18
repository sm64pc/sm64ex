#include <stdbool.h>

struct CheatList
{
    bool         EnableCheats;
	bool         MoonJump;
    bool         GodMode;
    bool         InfiniteLives;
    bool         SuperSpeed;
    bool         Responsive;
};

extern struct CheatList Cheats;
