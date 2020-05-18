#include <stdbool.h>

struct CheatList
{
    bool         EnableCheats;
	bool         MoonJump;
    bool         GodMode;
    bool         InfiniteLives;
    bool         SuperSpeed;
    bool         Responsive;
    bool         ExitAnywhere;
    bool         HugeMario;
    bool         TinyMario;
};

extern struct CheatList Cheats;
