#include "cliopts.h"

struct PCCLIOptions gCLIOpts;

void parse_cli_opts(int argc, char* argv[])
{
	// Initialize options with false values. 
	gCLIOpts.SkipIntro = 0;

	// Scan arguments for options
	if (argc > 1)
	{
		int i;
		for (i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "--skip-intro") == 0) // Skip Peach Intro
				gCLIOpts.SkipIntro = 1;
		}
	}
}