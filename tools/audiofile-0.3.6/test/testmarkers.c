/*
	Audio File Library

	Copyright (C) 2002, Silicon Graphics, Inc.

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <audiofile.h>

#include "TestUtilities.h"

static char sTestFileName[PATH_MAX];

#define FRAME_COUNT 200

void cleanup (void)
{
#ifndef DEBUG
	unlink(sTestFileName);
#endif
}

void ensure (int condition, const char *message)
{
	if (!condition)
	{
		printf("%s.\n", message);
		cleanup();
		exit(EXIT_FAILURE);
	}
}

int testmarkers (int fileformat)
{
	AFfilehandle	file;
	AFfilesetup	setup;
	int		markids[] = {1, 2, 3, 4};
	AFframecount	markpositions[] = {14, 54, 23, 101};
	const char	*marknames[] = {"one", "two", "three", "four"};
	short		frames[FRAME_COUNT * 2] = {0};
	int		readmarkcount;
	int		readmarkids[4];
	AFframecount	frameswritten;

	setup = afNewFileSetup();
	ensure(setup != AF_NULL_FILESETUP, "Could not create file setup");

	afInitFileFormat(setup, fileformat);
	afInitChannels(setup, AF_DEFAULT_TRACK, 2);

	afInitMarkIDs(setup, AF_DEFAULT_TRACK, markids, 4);

	afInitMarkName(setup, AF_DEFAULT_TRACK, markids[0], marknames[0]);
	afInitMarkName(setup, AF_DEFAULT_TRACK, markids[1], marknames[1]);
	afInitMarkName(setup, AF_DEFAULT_TRACK, markids[2], marknames[2]);
	afInitMarkName(setup, AF_DEFAULT_TRACK, markids[3], marknames[3]);

	file = afOpenFile(sTestFileName, "w", setup);
	ensure(file != AF_NULL_FILEHANDLE, "Could not open file for writing");

	afFreeFileSetup(setup);

	frameswritten = afWriteFrames(file, AF_DEFAULT_TRACK, frames, FRAME_COUNT);
	ensure(frameswritten == FRAME_COUNT, "Error writing audio data");

	afSetMarkPosition(file, AF_DEFAULT_TRACK, markids[0], markpositions[0]);
	afSetMarkPosition(file, AF_DEFAULT_TRACK, markids[1], markpositions[1]);
	afSetMarkPosition(file, AF_DEFAULT_TRACK, markids[2], markpositions[2]);
	afSetMarkPosition(file, AF_DEFAULT_TRACK, markids[3], markpositions[3]);

	afCloseFile(file);

	file = afOpenFile(sTestFileName, "r", NULL);
	ensure(file != AF_NULL_FILEHANDLE, "Could not open file for reading");

	readmarkcount = afGetMarkIDs(file, AF_DEFAULT_TRACK, NULL);
	ensure(readmarkcount == 4, "Number of markers is not correct");

	afGetMarkIDs(file, AF_DEFAULT_TRACK, readmarkids);

	for (int i=0; i<readmarkcount; i++)
		ensure(readmarkids[i] = markids[i],
			"Marker identification numbers do not match");

	for (int i=0; i<readmarkcount; i++)
	{
		AFframecount	readmarkposition;
		const char	*readmarkname;

		readmarkposition = afGetMarkPosition(file, AF_DEFAULT_TRACK, readmarkids[i]);

		readmarkname = afGetMarkName(file, AF_DEFAULT_TRACK, readmarkids[i]);

		ensure(readmarkposition == markpositions[i],
			"Marker positions do not match");
		ensure(strcmp(readmarkname, marknames[i]) == 0,
			"Marker names do not match");
	}

	afCloseFile(file);

	return EXIT_SUCCESS;
}

int main (void)
{
	ensure(createTemporaryFile("testmarkers", sTestFileName),
		"could not create temporary file");

	testmarkers(AF_FILE_AIFF);
	testmarkers(AF_FILE_AIFFC);
	testmarkers(AF_FILE_WAVE);

	cleanup();

	return EXIT_SUCCESS;
}
