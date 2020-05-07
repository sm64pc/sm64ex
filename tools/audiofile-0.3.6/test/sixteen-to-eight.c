/*
	Audio File Library

	Copyright 2000, Silicon Graphics, Inc.

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
	sixteen-to-eight.c

	This program tests the conversion from 16-bit integers to 8-bit
	integers.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <audiofile.h>

#include "TestUtilities.h"

int main (int argc, char **argv)
{
	AFfilehandle file;
	AFfilesetup setup;
	int16_t frames16[] = {14298, 392, 3923, -683, 958, -1921};
	int8_t frames8[] = {55, 1, 15, -3, 3, -8};
	int i, frameCount = 6;
	int8_t byte;
	AFframecount result;

	setup = afNewFileSetup();

	afInitFileFormat(setup, AF_FILE_WAVE);

	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_UNSIGNED, 8);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	char testFileName[PATH_MAX];
	if (!createTemporaryFile("sixteen-to-eight", testFileName))
	{
		fprintf(stderr, "Could not create temporary file.\n");
		exit(EXIT_FAILURE);
	}

	file = afOpenFile(testFileName, "w", setup);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "could not open file for writing\n");
		exit(EXIT_FAILURE);
	}

	afFreeFileSetup(setup);

	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);

	afWriteFrames(file, AF_DEFAULT_TRACK, frames16, frameCount);

	afCloseFile(file);

	file = afOpenFile(testFileName, "r", AF_NULL_FILESETUP);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "could not open file for reading\n");
		exit(EXIT_FAILURE);
	}

	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 8);

	for (i=0; i<frameCount; i++)
	{
		/* Read one frame. */
		result = afReadFrames(file, AF_DEFAULT_TRACK, &byte, 1);

		if (result != 1)
			break;

		/* Compare the byte read with its precalculated value. */
		if (memcmp(&byte, &frames8[i], 1) != 0)
		{
			printf("error\n");
			printf("expected %d, got %d\n", frames8[i], byte);
			exit(EXIT_FAILURE);
		}
		else
		{
#ifdef DEBUG
			printf("got what was expected: %d\n", byte);
#endif
		}
	}

	afCloseFile(file);
	unlink(testFileName);

	exit(EXIT_SUCCESS);
}
