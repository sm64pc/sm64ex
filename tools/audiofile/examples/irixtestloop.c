/*
	Audio File Library

	Copyright (C) 1998-1999, Michael Pruett <michael@68k.org>
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
	irixtestloop.c

	This file reads the loop points from a file (presumably AIFF) and
	loops that part of the file several times.  Audio output is routed
	to IRIX's default audio output device.  This program will not
	compile on any platform other than IRIX.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dmedia/audio.h>
#include <dmedia/audiofile.h>

#include "sgi.h"

const int REPEAT_COUNT = 3;

void usage (void)
{
	printf("usage: irixtestloop file\n");
	printf("where file is of a format which contains a loop (e.g. AIFF)\n");
	exit(EXIT_FAILURE);
}

main (int argc, char **argv)
{
	AFfilehandle	file;
	void		*buffer;

	AFframecount	frameCount;
	int		sampleFormat, sampleWidth, channelCount;
	float		frameSize;
	double		sampleRate;

	int		*loopids, *markids;
	int		i, loopCount, markCount;
	int		startmarkid, endmarkid;
	AFframecount	startloop, endloop;

	ALport		outport;
	ALconfig	outportconfig;

	if (argc < 2)
		usage();

	file = afOpenFile(argv[1], "r", NULL);
	frameCount = afGetFrameCount(file, AF_DEFAULT_TRACK);
	frameSize = afGetVirtualFrameSize(file, AF_DEFAULT_TRACK, 1);
	channelCount = afGetVirtualChannels(file, AF_DEFAULT_TRACK);
	afGetVirtualSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	sampleRate = afGetRate(file, AF_DEFAULT_TRACK);

	/*
		If the file's sample format is unsigned integer data,
		change the virtual sample format to two's complement
		since the SGI Audio Library won't accept unsigned
		data.
	*/
	if (sampleFormat == AF_SAMPFMT_UNSIGNED)
	{
		afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK,
			AF_SAMPFMT_TWOSCOMP, sampleWidth);
	}

	printf("frame count: %lld\n", frameCount);
	printf("frame size: %d bytes\n", frameSize);
	printf("channel count: %d\n", channelCount);
	printf("sample rate: %.2f Hz\n", sampleRate);

	buffer = malloc(frameCount * frameSize);
	afReadFrames(file, AF_DEFAULT_TRACK, buffer, frameCount);

	loopCount = afGetLoopIDs(file, AF_DEFAULT_INST, NULL);
	loopids = malloc(sizeof (int) * loopCount);
	afGetLoopIDs(file, AF_DEFAULT_INST, loopids);

	markCount = afGetMarkIDs(file, AF_DEFAULT_TRACK, NULL);
	markids = malloc(sizeof (int) * markCount);
	afGetMarkIDs(file, AF_DEFAULT_TRACK, markids);

	printf("loop ids:");
	for (i=0; i<loopCount; i++)
		printf(" %d", loopids[i]);
	printf("\n");

	printf("mark ids:");
	for (i=0; i<markCount; i++)
		printf(" %d", markids[i]);
	printf("\n");

	startmarkid = afGetLoopStart(file, AF_DEFAULT_INST, 1);
	endmarkid = afGetLoopEnd(file, AF_DEFAULT_INST, 1);
	startloop = afGetMarkPosition(file, AF_DEFAULT_TRACK, startmarkid);
	endloop = afGetMarkPosition(file, AF_DEFAULT_TRACK, endmarkid);

	afCloseFile(file);

	outportconfig = alNewConfig();
	setwidth(outportconfig, sampleWidth);
	setsampleformat(outportconfig, sampleFormat);
	alSetChannels(outportconfig, channelCount);

	outport = alOpenPort("irixtestloop", "w", outportconfig);
	setrate(outport, sampleRate);

	alWriteFrames(outport, buffer, startloop);
	for (i=0; i<REPEAT_COUNT; i++)
	{
		printf("iteration %d: start %lld, end %lld, length %lld\n",
			i, endloop, startloop, endloop - startloop);
		alWriteFrames(outport,
			(char *) buffer + (int) (startloop * frameSize),
			endloop - startloop);
	}

	waitport(outport);

	alClosePort(outport);
	alFreeConfig(outportconfig);

	free(buffer);
}
