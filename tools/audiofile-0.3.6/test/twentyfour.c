/*
	Audio File Library

	Copyright (C) 2001, Silicon Graphics, Inc.
	Michael Pruett <mpruett@sgi.com>

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
	twentyfour.c

	This program tests the conversion between 24-bit signed integer
	data in a file and 32-bit signed integer data in memory.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <audiofile.h>

#include "TestUtilities.h"

#define FRAME_COUNT 6

int main (int argc, char **argv)
{
	AFfilehandle file;
	AFfilesetup setup;
	/* All elements in frames32 must be in the range -2^23 to 2^23 - 1. */
	const int32_t frames32[FRAME_COUNT] =
		{4314298, -49392, 3923, -143683, 43, -992129};
	const uint8_t frames24[FRAME_COUNT*3] =
		{
			0x41, 0xd4, 0xba,	/* 4314298 */
			0xff, 0x3f, 0x10,	/* -49392 */
			0x00, 0x0f, 0x53,	/* 3923 */
			0xfd, 0xce, 0xbd,	/* -143683 */
			0x00, 0x00, 0x2b,	/* 43 */
			0xf0, 0xdc, 0x7f	/* -992129 */
		};
	int32_t readframes32[FRAME_COUNT];
	int i;

	setup = afNewFileSetup();
	assert(setup);

	afInitFileFormat(setup, AF_FILE_AIFF);

	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 24);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	char testFileName[PATH_MAX];
	if (!createTemporaryFile("twentyfour", testFileName))
	{
		fprintf(stderr, "could not create temporary file\n");
		exit(EXIT_FAILURE);
	}

	file = afOpenFile(testFileName, "w", setup);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "could not open file for writing\n");
		exit(EXIT_FAILURE);
	}

	afFreeFileSetup(setup);

	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 24);
	afWriteFrames(file, AF_DEFAULT_TRACK, frames32, FRAME_COUNT);

	afCloseFile(file);

	file = afOpenFile(testFileName, "r", AF_NULL_FILESETUP);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "could not open file for reading\n");
		exit(EXIT_FAILURE);
	}

	/* Test virtual sample width of 24 bits. */
#ifdef DEBUG
	fprintf(stderr, "Testing virtual sample width of 24 bits.\n");
#endif
	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 24);

	for (i=0; i<FRAME_COUNT; i++)
	{
		uint8_t	x[4];
		uint8_t	y[4];
		uint8_t	z[4];

		if ((frames32[i] & 0x800000) != 0)
			x[0] = 0xff;
		else
			x[0] = 0;
		x[1] = (frames32[i] >> 16) & 0xff;
		x[2] = (frames32[i] >> 8) & 0xff;
		x[3] = (frames32[i]) & 0xff;

		/*
			Check to see that the precomputed values match
			what we've just computed.
		*/
		if (x[1] != frames24[3*i] ||
			x[2] != frames24[3*i + 1] ||
			x[3] != frames24[3*i + 2])
		{
			fprintf(stderr, "Data doesn't match pre-computed values.\n");
			exit(EXIT_FAILURE);
		}

		if (afReadFrames(file, AF_DEFAULT_TRACK, y, 1) != 1)
		{
			fprintf(stderr, "Could not read from test file.\n");
			exit(EXIT_FAILURE);
		}

		/*
			x is in big-endian byte order; make z a
			native-endian copy of x.
		*/
#ifdef WORDS_BIGENDIAN
		memcpy(z, x, 4);
#else
		z[0] = x[3];
		z[1] = x[2];
		z[2] = x[1];
		z[3] = x[0];
#endif

#ifdef DEBUG
		printf("x = %02x %02x %02x %02x\n", x[0], x[1], x[2], x[3]);
		printf("y = %02x %02x %02x %02x\n", y[0], y[1], y[2], y[3]);
		printf("z = %02x %02x %02x %02x\n", z[0], z[1], z[2], z[3]);
#endif

		/*
			Check to see that the data read from the file
			matches computed value.
		*/
		if (memcmp(y, z, 4) != 0)
		{
			fprintf(stderr, "Data read from file is incorrect.\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Test virtual sample width of 32 bits. */
#ifdef DEBUG
	fprintf(stderr, "Testing virtual sample width of 32 bits.\n");
#endif
	afSeekFrame(file, AF_DEFAULT_TRACK, 0);
	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 32);

	for (i=0; i<FRAME_COUNT; i++)
	{
		uint8_t	x[4];
		uint8_t	y[4];
		uint8_t	z[4];

		x[0] = (frames32[i] >> 16) & 0xff;
		x[1] = (frames32[i] >> 8) & 0xff;
		x[2] = (frames32[i]) & 0xff;
		x[3] = 0;

		/*
			Check to see that the precomputed values match
			what we've just computed.
		*/
		if (x[0] != frames24[3*i] ||
			x[1] != frames24[3*i + 1] ||
			x[2] != frames24[3*i + 2])
		{
			fprintf(stderr, "Data doesn't match pre-computed values.\n");
			exit(EXIT_FAILURE);
		}

		if (afReadFrames(file, AF_DEFAULT_TRACK, y, 1) != 1)
		{
			fprintf(stderr, "Could not read from test file.\n");
			exit(EXIT_FAILURE);
		}

		/*
			x is in big-endian byte order; make z a
			native-endian copy of x.
		*/
#ifdef WORDS_BIGENDIAN
		memcpy(z, x, 4);
#else
		z[0] = x[3];
		z[1] = x[2];
		z[2] = x[1];
		z[3] = x[0];
#endif

#ifdef DEBUG
		printf("x = %02x %02x %02x %02x\n", x[0], x[1], x[2], x[3]);
		printf("y = %02x %02x %02x %02x\n", y[0], y[1], y[2], y[3]);
		printf("z = %02x %02x %02x %02x\n", z[0], z[1], z[2], z[3]);
#endif

		/*
			Check to see that the data read from the file
			matches computed value.
		*/
		if (memcmp(y, z, 4) != 0)
		{
			fprintf(stderr, "Data read from file is incorrect.\n");
			exit(EXIT_FAILURE);
		}
	}

	if (afCloseFile(file) != 0)
	{
		fprintf(stderr, "Error closing file.\n");
		exit(EXIT_FAILURE);
	}
	unlink(testFileName);

	exit(EXIT_SUCCESS);
}
