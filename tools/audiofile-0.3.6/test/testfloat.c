/*
	Audio File Library

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
	testfloat.c

	This program tests floating-point reading and writing for
	the AIFF-C, WAVE, NeXT .snd, and IRCAM file formats.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <audiofile.h>

#include "TestUtilities.h"

static char sTestFileName[PATH_MAX];

const float samples[] =
	{1.0, 0.6, -0.3, 0.95, 0.2, -0.6, 0.9, 0.4, -0.22, 0.125, 0.1, -0.4};
#define SAMPLE_COUNT (sizeof (samples) / sizeof (float))

void testfloat (int fileFormat);

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

int main (int argc, char **argv)
{
	/* These file formats support floating-point audio data. */
	int	fileFormatCount = 4;
	int	fileFormats[] =
		{AF_FILE_AIFFC, AF_FILE_WAVE, AF_FILE_NEXTSND, AF_FILE_IRCAM};
	char	*formatNames[] = {"AIFF-C", "WAVE", "NeXT .snd", "IRCAM"};
	int	i;

	for (i=0; i<fileFormatCount; i++)
	{
		printf("testfloat: testing %s\n", formatNames[i]);
		testfloat(fileFormats[i]);
	}

	printf("testfloat passed\n");
	exit(EXIT_SUCCESS);
}

void testfloat (int fileFormat)
{
	AFfilesetup	setup;
	AFfilehandle	file;
	int		framesWritten, framesRead;
	const int 	frameCount = SAMPLE_COUNT/2;
	float		readsamples[SAMPLE_COUNT];
	int		i;
	int		sampleFormat, sampleWidth;

	setup = afNewFileSetup();

	afInitFileFormat(setup, fileFormat);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_FLOAT, 32);
	afInitChannels(setup, AF_DEFAULT_TRACK, 2);

	ensure(createTemporaryFile("testfloat", sTestFileName),
		"could not create temporary file");
	file = afOpenFile(sTestFileName, "w", setup);
	ensure(file != AF_NULL_FILEHANDLE, "could not open file for writing");

	afFreeFileSetup(setup);

	framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, samples,
		frameCount);
	ensure(framesWritten == frameCount, "number of frames written does not match number of frames requested");

	ensure(afCloseFile(file) == 0, "error closing file");

	file = afOpenFile(sTestFileName, "r", AF_NULL_FILESETUP);
	ensure(file != AF_NULL_FILEHANDLE, "could not open file for reading");

	ensure(afGetChannels(file, AF_DEFAULT_TRACK) == 2,
		"file doesn't have exactly two channels");
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	ensure(sampleFormat == AF_SAMPFMT_FLOAT && sampleWidth == 32,
		"file doesn't contain 32-bit floating-point data");
	ensure(afGetFileFormat(file, NULL) == fileFormat,
		"file format doesn't match format requested");

	framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readsamples,
		frameCount);
	ensure(framesRead == frameCount, "number of frames read does not match number of frames requested");

	for (i=0; i<SAMPLE_COUNT; i++)
	{
		ensure(readsamples[i] == samples[i],
			"data written to file doesn't match data read");
	}

	ensure(afCloseFile(file) == 0, "error closing file");

	cleanup();
}
