/*
	Audio File Library

	Copyright 1998, Michael Pruett <michael@68k.org>

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
	adddcoffset.c

	This program adds a user-specified DC offset term to a sound file.
	It's not very useful.
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include "audiofile.h"
#endif

void adddcoffset (float offset, char *infilename, char *outfilename);
void usageerror (void);

int main (int ac, char **av)
{
	float	offset;

	if (ac != 4)
		usageerror();

	offset = atof(av[1]);
	adddcoffset(offset, av[2], av[3]);
	return 0;
}

void adddcoffset (float offset, char *infilename, char *outfilename)
{
	AFfilehandle	infile = afOpenFile(infilename, "r", NULL);
	int				channelCount, frameCount;
	short			*buffer;
	int				i;

	AFfilesetup		outfilesetup = afNewFileSetup();
	AFfilehandle	outfile;

	frameCount = afGetFrameCount(infile, AF_DEFAULT_TRACK);
	channelCount = afGetChannels(infile, AF_DEFAULT_TRACK);

	afInitFileFormat(outfilesetup, AF_FILE_AIFF);
	afInitByteOrder(outfilesetup, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
	afInitChannels(outfilesetup, AF_DEFAULT_TRACK, channelCount);
	afInitRate(outfilesetup, AF_DEFAULT_TRACK, 44100.0);
	afInitSampleFormat(outfilesetup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);

	outfile = afOpenFile(outfilename, "w", outfilesetup);

	if (infile == NULL)
	{
		printf("unable to open input file '%s'.\n", infilename);
		return;
	}

	printf("afGetFrameCount: %d\n", (int)afGetFrameCount(infile, AF_DEFAULT_TRACK));
	printf("afGetChannels: %d\n", afGetChannels(infile, AF_DEFAULT_TRACK));

	buffer = (short *) malloc(frameCount * channelCount * sizeof (short));
	afReadFrames(infile, AF_DEFAULT_TRACK, (void *) buffer, frameCount);

	for (i=0; i<frameCount; i++)
	{
		buffer[i] += offset;
	}

	afWriteFrames(outfile, AF_DEFAULT_TRACK, (void *) buffer, frameCount);
	afCloseFile(outfile);
}

void usageerror (void)
{
	fprintf(stderr, "usage: adddcoffset offset infile outfile\n");
	exit(EXIT_FAILURE);
}
