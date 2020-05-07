/*
	Audio File Library

	Copyright (C) 2003, Michael Pruett <michael@68k.org>

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
	twentyfour2.c

	This program checks reading and writing a large amount of 24-bit
	audio data to an AIFF file.

	This program serves as a regression test for a bug in the Audio
	File Library in which requesting more than _AF_ATOMIC_NVFRAMES
	(1024 frames) from afReadFrames when reading a 24-bit audio file
	would result in corrupted audio data.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <audiofile.h>

#include "TestUtilities.h"

static char sTestFileName[PATH_MAX];

#define FRAME_COUNT 10000

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

int main (void)
{
	AFfilehandle	file;
	AFfilesetup	setup;
	int32_t		*buffer, *readbuffer;
	int		i;
	AFframecount	frameswritten, framesread;

	setup = afNewFileSetup();
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 24);

	ensure(createTemporaryFile("twentyfour2", sTestFileName),
		"could not create temporary file");
	file = afOpenFile(sTestFileName, "w", setup);
	ensure(file != NULL, "could not open test file for writing");

	afFreeFileSetup(setup);

	buffer = malloc(sizeof (int32_t) * FRAME_COUNT);
	ensure(buffer != NULL, "could not allocate buffer for audio data");

	readbuffer = malloc(sizeof (int32_t) * FRAME_COUNT);
	ensure(readbuffer != NULL, "could not allocate buffer for audio data");

	for (i=0; i<FRAME_COUNT; i++)
	{
		if ((i%3) == 0)
			buffer[i] = -i;
		else
			buffer[i] = i;
	}

	frameswritten = afWriteFrames(file, AF_DEFAULT_TRACK, buffer, FRAME_COUNT);
	ensure(frameswritten == FRAME_COUNT, "incorrect number of frames written");

	afCloseFile(file);

	/*
		Now open file for reading and ensure that the data read
		is equal to the data written.
	*/
	file = afOpenFile(sTestFileName, "r", AF_NULL_FILESETUP);
	ensure(file != NULL, "could not open test file for reading");

	framesread = afReadFrames(file, AF_DEFAULT_TRACK, readbuffer, FRAME_COUNT);
	ensure(framesread == FRAME_COUNT, "incorrect number of frames read");

#ifdef DEBUG
	for (i=0; i<FRAME_COUNT; i++)
	{
		if (buffer[i] != readbuffer[i])
		{
			printf("buffer[%d] = %d, readbuffer[%d] = %d\n",
				i, buffer[i], i, readbuffer[i]);
		}
	}
#endif

	ensure(!memcmp(buffer, readbuffer, sizeof (int32_t) * FRAME_COUNT),
		"data read does not match data written");

	afCloseFile(file);

	free(buffer);
	free(readbuffer);

	cleanup();
	return 0;
}
