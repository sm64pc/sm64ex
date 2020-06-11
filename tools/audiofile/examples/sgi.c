/*
	Audio File Library

	Copyright 1998-1999, Michael Pruett <michael@68k.org>

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
	sgi.c

	These routines are used in SGI-specific test programs.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <audiofile.h>

#include <dmedia/audio.h>

/*
	Set the sample width of an audio configuration.
*/
void setwidth (ALconfig config, int sampleWidth)
{
	if (sampleWidth <= 8)
	{
		printf("setting width to 8\n");
		alSetWidth(config, AL_SAMPLE_8);
	}
	else if (sampleWidth <= 16)
	{
		printf("setting width to 16\n");
		alSetWidth(config, AL_SAMPLE_16);
	}
	else if (sampleWidth <= 24)
	{
		printf("setting width to 24\n");
		alSetWidth(config, AL_SAMPLE_24);
	}
}

/*
	Set the sample format of an audio configuration.
*/
void setsampleformat (ALconfig config, int audioFileSampleFormat)
{
	if (audioFileSampleFormat == AF_SAMPFMT_TWOSCOMP)
	{
		printf("setting sample format to 2's complement\n");
		alSetSampFmt(config, AL_SAMPFMT_TWOSCOMP);
	}
	else if (audioFileSampleFormat == AF_SAMPFMT_FLOAT)
	{
		printf("setting sample format to float\n");
		alSetSampFmt(config, AL_SAMPFMT_FLOAT);
	}
	else if (audioFileSampleFormat == AF_SAMPFMT_DOUBLE)
	{
		printf("setting sample format to double\n");
		alSetSampFmt(config, AL_SAMPFMT_DOUBLE);
	}
}

/*
	Set the sample rate of an audio port.
*/
void setrate (ALport port, double rate)
{
	int		rv;
	ALpv	params;

	rv = alGetResource(port);

	params.param = AL_RATE;
	params.value.ll = alDoubleToFixed(rate);

	if (alSetParams(rv, &params, 1) < 0)
	{
		printf("alSetParams failed: %s\n", alGetErrorString(oserror()));
	}
}

/*
	Wait until the audio port has no more samples to play.
*/
void waitport (ALport port)
{
	while (alGetFilled(port) > 0)
		sginap(1);
}
