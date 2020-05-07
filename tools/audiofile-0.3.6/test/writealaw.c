/*
	Audio File Library

	Copyright (C) 2000, Michael Pruett <michael@68k.org>
	Copyright (C) 2001, Silicon Graphics, Inc.

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
	writealaw.c

	The writealaw program performs sanity testing on the Audio File
	Library's G.711 A-law compression by writing and then reading
	back known data to a file to make sure the two sets of data agree.

	This program writes a set of data which is invariant under G.711
	A-law compression to a file and then reads that set of data back.

	The data read from that file should match the data written
	exactly.

	If this test fails, something in the Audio File Library is broken.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include <audiofile.h>
#endif

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "TestUtilities.h"

static char sTestFileName[PATH_MAX];

#define FRAME_COUNT 16
#define SAMPLE_COUNT FRAME_COUNT

void testalaw (int fileFormat);

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

int main (int argc, char **argv)
{
	printf("writealaw: testing NeXT .snd.\n");
	testalaw(AF_FILE_NEXTSND);
	printf("writealaw: testing AIFF-C.\n");
	testalaw(AF_FILE_AIFFC);
	printf("writealaw: testing WAVE.\n");
	testalaw(AF_FILE_WAVE);
	printf("writealaw: testing IRCAM.\n");
	testalaw(AF_FILE_IRCAM);
	printf("writealaw: testing VOC.\n");
	testalaw(AF_FILE_VOC);
	printf("writealaw: testing CAF.\n");
	testalaw(AF_FILE_CAF);

	printf("writealaw test passed.\n");

	exit(0);
}

void testalaw (int fileFormat)
{
	AFfilehandle	file;
	AFfilesetup	setup;
	uint16_t	samples[] = {8, 24, 88, 120, 184, 784, 912, 976,
                        1120, 1440, 1888, 8960, 9984, 16128, 19968, 32256};
	uint16_t	readsamples[SAMPLE_COUNT];
	AFframecount	framesWritten, framesRead;
	int		i;

	setup = afNewFileSetup();

	afInitCompression(setup, AF_DEFAULT_TRACK, AF_COMPRESSION_G711_ALAW);
	afInitFileFormat(setup, fileFormat);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	ensure(createTemporaryFile("writealaw", sTestFileName),
		"could not create temporary file");
	file = afOpenFile(sTestFileName, "w", setup);
	afFreeFileSetup(setup);

	ensure(afGetCompression(file, AF_DEFAULT_TRACK) ==
		AF_COMPRESSION_G711_ALAW,
		"test file not created with G.711 A-law compression");

	ensure(file != AF_NULL_FILEHANDLE, "unable to open file for writing");

	framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, samples,
		FRAME_COUNT);

	ensure(framesWritten == FRAME_COUNT,
		"number of frames requested does not match number of frames written");
	afCloseFile(file);

	/* Open the file for reading and verify the data. */
	file = afOpenFile(sTestFileName, "r", NULL);
	ensure(file != AF_NULL_FILEHANDLE, "unable to open file for reading");

	ensure(afGetFileFormat(file, NULL) == fileFormat,
		"test file format incorrect");

	ensure(afGetCompression(file, AF_DEFAULT_TRACK) ==
		AF_COMPRESSION_G711_ALAW,
		"test file not opened with G.711 A-law compression");

	framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readsamples,
		FRAME_COUNT);

	ensure(framesRead == FRAME_COUNT,
		"number of frames read does not match number of frames requested");

#ifdef DEBUG
	for (i=0; i<SAMPLE_COUNT; i++)
		printf("readsamples[%d]: %d\n", i, readsamples[i]);
	for (i=0; i<SAMPLE_COUNT; i++)
		printf("samples[%d]: %d\n", i, samples[i]);
#endif

	for (i=0; i<SAMPLE_COUNT; i++)
	{
		ensure(samples[i] == readsamples[i],
			"data written does not match data read");
	}

	/* G.711 compression uses one byte per sample. */
	ensure(afGetTrackBytes(file, AF_DEFAULT_TRACK) == SAMPLE_COUNT,
		"track byte count is incorrect");

	ensure(afGetFrameCount(file, AF_DEFAULT_TRACK) == FRAME_COUNT,
		"frame count is incorrect");

	ensure(afGetChannels(file, AF_DEFAULT_TRACK) == 1,
		"channel count is incorrect");

	ensure(afCloseFile(file) == 0, "error closing file");

	cleanup();
}
