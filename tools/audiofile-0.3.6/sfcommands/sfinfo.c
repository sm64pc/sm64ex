/*
	Audio File Library

	Copyright 1998, 2011, Michael Pruett <michael@68k.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
	sfinfo.c

	This program displays information about audio files.
*/

#include "config.h"

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include <audiofile.h>
#endif

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printinfo.h"

bool reportError = false;

void errorHandler(long error, const char *message)
{
	if (reportError)
		fprintf(stderr, "sfinfo: %s [error %ld]\n", message, error);
}

void printusage()
{
	printf("usage: sfinfo [options...] soundfiles...\n");
	printf("options:\n");
	printf("  -s, --short        Print information in short format\n");
	printf("  -r, --reporterror  Report errors when reading sound files\n");
	printf("  -h, --help         Print this help message\n");
	printf("  -v, --version      Print version\n");
}

void printversion()
{
	printf("sfinfo: Audio File Library version %s\n", VERSION);
}

int main(int argc, char **argv)
{
	bool brief = false;

	afSetErrorHandler(errorHandler);

	if (argc == 1)
	{
		printusage();
		return 0;
	}

	static struct option long_options[] =
	{
		{"short", 0, 0, 's'},
		{"reporterror", 0, 0, 'r'},
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
		{0, 0, 0, 0}
	};

	int result;
	int option_index = 1;
	while ((result = getopt_long(argc, argv, "srhv", long_options,
		&option_index)) != -1)
	{
		switch (result)
		{
			case 's':
				brief = true;
				break;
			case 'r':
				reportError = true;
				break;
			case 'h':
				printusage();
				exit(EXIT_SUCCESS);
			case 'v':
				printversion();
				exit(EXIT_SUCCESS);
		}
	}

	int i = optind;
	while (i < argc)
	{
		bool processed = brief ? printshortinfo(argv[i]) :
			printfileinfo(argv[i]);
		i++;
		if (!brief && processed && i < argc)
			putchar('\n');
	}

	return 0;
}
