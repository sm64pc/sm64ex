/*
	Audio File Library

	Copyright (C) 2003, Silicon Graphics, Inc.

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
	testchannelmatrix.c

	This program tests the channel matrix functionality of virtual
	sample format conversion in the Audio File Library.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <audiofile.h>

#include "TestUtilities.h"

static char sTestFileName[PATH_MAX];

const short samples[] = {300, -300, 515, -515, 2315, -2315, 9154, -9154};
#define SAMPLE_COUNT (sizeof (samples) / sizeof (short))
#define CHANNEL_COUNT 2

void cleanup (void)
{
	unlink(sTestFileName);
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

int main (void)
{
	AFfilesetup	setup;
	AFfilehandle	file;
	int		framesWritten, framesRead;
	const int 	frameCount = SAMPLE_COUNT / CHANNEL_COUNT;
	short		readsamples[SAMPLE_COUNT];
	int		i;
	int		sampleFormat, sampleWidth;

	setup = afNewFileSetup();

	afInitChannels(setup, AF_DEFAULT_TRACK, CHANNEL_COUNT);
	afInitFileFormat(setup, AF_FILE_AIFFC);

	/* Write stereo data to test file. */
	ensure(createTemporaryFile("testchannelmatrix", sTestFileName),
		"could not create temporary file");
	file = afOpenFile(sTestFileName, "w", setup);
	ensure(file != AF_NULL_FILEHANDLE, "could not open file for writing");

	afFreeFileSetup(setup);

	framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, samples,
		frameCount);
	ensure(framesWritten == frameCount,
		"number of frames written doesn't match "
		"number of frames requested");

	ensure(afCloseFile(file) == 0, "error closing file");

	/*
		Open the test file and read stereo data mixed down to a
		single channel.  The default channel matrix for one
		file channel and two virtual channels is {0.5, 0.5},
		and since each odd sample is the inverse of the
		corresponding even sample, the data read should be all
		zeros.
	*/
	file = afOpenFile(sTestFileName, "r", AF_NULL_FILESETUP);
	ensure(file != AF_NULL_FILEHANDLE, "could not open file for reading");

	ensure(afGetChannels(file, AF_DEFAULT_TRACK) == 2,
		"file doesn't have exactly two channels");
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	ensure(sampleFormat == AF_SAMPFMT_TWOSCOMP && sampleWidth == 16,
		"file doesn't contain 16-bit two's complement data");
	ensure(afGetFileFormat(file, NULL) == AF_FILE_AIFFC,
		"file format doesn't match format requested");

	afSetVirtualChannels(file, AF_DEFAULT_TRACK, 1);

	framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readsamples,
		frameCount);
	ensure(framesRead == frameCount, "number of frames read does not match number of frames requested");

	for (i=0; i<SAMPLE_COUNT/CHANNEL_COUNT; i++)
	{
		ensure(readsamples[i] == 0,
			"data written to file is not as expected");
	}

	ensure(afCloseFile(file) == 0, "error closing file");

	cleanup();
	exit(EXIT_SUCCESS);
}
