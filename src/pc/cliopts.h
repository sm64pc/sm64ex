#include "sm64.h"

struct PCCLIOptions 
{
	u8 SkipIntro;
    u8 FullScreen;
};

extern struct PCCLIOptions gCLIOpts;

void parse_cli_opts(int argc, char* argv[]);
