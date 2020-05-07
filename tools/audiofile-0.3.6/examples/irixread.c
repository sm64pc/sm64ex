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
	irixread.c

	This program reads and plays a given audio file using Irix's
	default audio output device.  This file will not work on any
	operating system other than Irix.

	The only difference between this program and irixtest is that this
	program does not load the entire audio track into memory at once.
	Only a small number of frames are read into a buffer and then
	written to the audio port.  While there are more frames to be
	read, this process is repeated.
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

const int BUFFERED_FRAME_COUNT = 65536;

void usage (void)
{
	fprintf(stderr, "usage: irixread filename\n");
	exit(EXIT_FAILURE);
}

main (int argc, char **argv)
{
	AFfilehandle	file;
	AFframecount	count, frameCount;
	int		channelCount, sampleFormat, sampleWidth;
	float		frameSize;
	void		*buffer;
	double		sampleRate;

	ALport		outport;
	ALconfig	outportconfig;

	if (argc < 2)
		usage();

	file = afOpenFile(argv[1], "r", NULL);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "Could not open file %s.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	frameCount = afGetFrameCount(file, AF_DEFAULT_TRACK);
	frameSize = afGetVirtualFrameSize(file, AF_DEFAULT_TRACK, 1);
	channelCount = afGetVirtualChannels(file, AF_DEFAULT_TRACK);
	sampleRate = afGetRate(file, AF_DEFAULT_TRACK);
	afGetVirtualSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat,
		&sampleWidth);

	if (sampleFormat == AF_SAMPFMT_UNSIGNED)
	{
		afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK,
			AF_SAMPFMT_TWOSCOMP, sampleWidth);
	}

	printf("frame count: %lld\n", frameCount);
	printf("frame size: %d bytes\n", (int) frameSize);
	printf("channel count: %d\n", channelCount);
	printf("sample rate: %.2f Hz\n", sampleRate);
	buffer = malloc(BUFFERED_FRAME_COUNT * frameSize);

	outportconfig = alNewConfig();
	setwidth(outportconfig, sampleWidth);
	setsampleformat(outportconfig, sampleFormat);
	alSetChannels(outportconfig, channelCount);

	count = afReadFrames(file, AF_DEFAULT_TRACK, buffer, BUFFERED_FRAME_COUNT);

	outport = alOpenPort("irixread", "w", outportconfig);
	setrate(outport, sampleRate);

	do
	{
		printf("count = %lld\n", count);
		alWriteFrames(outport, buffer, count);

		count = afReadFrames(file, AF_DEFAULT_TRACK, buffer,
			BUFFERED_FRAME_COUNT);
	} while (count > 0);

	waitport(outport);

	alClosePort(outport);
	alFreeConfig(outportconfig);

	afCloseFile(file);
}
