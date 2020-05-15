#include "cliopts.h"
#include <strings.h>

struct PCCLIOptions gCLIOpts;

void parse_cli_opts(int argc, char* argv[])
{
	// Initialize options with false values. 
	gCLIOpts.SkipIntro  = 0;
    gCLIOpts.FullScreen = 0;

	// Scan arguments for options
	if (argc > 1)
	{
		int i;
		for (i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "--skip-intro") == 0) // Skip Peach Intro
				gCLIOpts.SkipIntro = 1;

            if (strcmp(argv[i], "--fullscreen") == 0) // Open game in fullscreen
                gCLIOpts.FullScreen = 1;            
		}
	}
}