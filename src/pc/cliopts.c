#include "cliopts.h"
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct PCCLIOptions gCLIOpts;

void parse_cli_opts(int argc, char* argv[])
{
	// Initialize options with false values. 
    gCLIOpts.SkipIntro   = 0;
    gCLIOpts.FullScreen  = 0;
    gCLIOpts.LevelSelect = 0;
    gCLIOpts.Profiler    = 0;
    gCLIOpts.Debug       = 0;
    gCLIOpts.ConfigFile = malloc(31);
    strncpy(gCLIOpts.ConfigFile, "sm64config.txt", strlen("sm64config.txt"));
    gCLIOpts.ConfigFile[strlen("sm64config.txt")] = '\0';

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

			if (strcmp(argv[i], "--windowed") == 0) // Open game in windowed mode
				gCLIOpts.FullScreen = 2;

            if (strcmp(argv[i], "--levelselect") == 0) // Open game in fullscreen
                gCLIOpts.LevelSelect = 1;

            if (strcmp(argv[i], "--profiler") == 0) // Open game in fullscreen
                gCLIOpts.Profiler = 1;

            if (strcmp(argv[i], "--debug") == 0) // Open game in fullscreen
                gCLIOpts.Debug = 1;

			if (strcmp(argv[i], "--help") == 0) // Print help
			{
				printf("Super Mario 64 PC Port\n");
				printf("%-20s\tSkips the Peach and Castle intro when starting a new game.\n", "--skip-intro");
				printf("%-20s\tStarts the game in full screen mode.\n", "--fullscreen");
				printf("%-20s\tStarts the game in windowed mode.\n", "--windowed");
				printf("%-20s\tOffers the user a level select menu.\n", "--levelselect");
				printf("%-20s\tEnables profiler bar on the screen bottom.\n", "--profiler");
				printf("%-20s\tEnables simple debug display.\n", "--debug");
				printf("%-20s\tSaves the configuration file as CONFIGNAME.\n", "--configfile CONFIGNAME");
				exit(0);
			}

            if (strncmp(argv[i], "--configfile", strlen("--configfile")) == 0)
            {
				if (i+1 < argc)
				{
					if (strlen(argv[i]) > 30) {
						fprintf(stderr, "Configuration file supplied has a name too long.\n");
					} else {
						memset(gCLIOpts.ConfigFile, 0, 30);
						strncpy(gCLIOpts.ConfigFile, argv[i+1], strlen(argv[i+1]));
						gCLIOpts.ConfigFile[strlen(argv[i+1])] = '\0';
					}
				}
			}
		}
	}
}
