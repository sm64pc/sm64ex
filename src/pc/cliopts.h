#include "sm64.h"

struct PCCLIOptions 
{
	u8 SkipIntro;
    u8 FullScreen;
    u8 LevelSelect;
    u8 Profiler;
    u8 Debug;
    char * ConfigFile;
};

extern struct PCCLIOptions gCLIOpts;

void parse_cli_opts(int argc, char* argv[]);
