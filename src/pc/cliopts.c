#include "cliopts.h"
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

struct PCCLIOptions gCLIOpts;

void parse_cli_opts(int argc, char* argv[])
{
	// Initialize options with false values. 
	gCLIOpts.SkipIntro  = 0;
    gCLIOpts.FullScreen = 0;
    gCLIOpts.ConfigFile = malloc(31);
    strncpy(gCLIOpts.ConfigFile, "sm64config.txt", strlen("sm64config.txt"));

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

            if (strncmp(argv[i], "--configfile", strlen("--configfile")) == 0)
            {
				if (i+1 < argc)
				{
					if (strlen(argv[i]) > 30) {
						fprintf(stderr, "Configuration file supplied has a name too long.\n");
					} else {
						memset(gCLIOpts.ConfigFile, 0, 30);
						strncpy(gCLIOpts.ConfigFile, argv[i+1], strlen(argv[i+1]));
					}
				}
			}	
		}
	}
}