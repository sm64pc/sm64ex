/*
	This program is derived from Chris Vaill's normalize program
	and has been modified to use the Audio File Library for file
	reading and audio data conversion.

	Copyright (C) 2001, Silicon Graphics, Inc.
	Copyright (C) 1999-2001, Chris Vaill

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
	power.c

	Calculate the power and peak amplitudes of an audio file.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <audiofile.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct smooth
{
	double	*buf;
	int	length, start, n;
};

static double get_smoothed_data (struct smooth *s)
{
	int i;
	double smoothed;

	smoothed = 0;
	for (i = 0; i < s->n; i++)
		smoothed += s->buf[i];
	smoothed = smoothed / s->n;

	return smoothed;
}

void print_power (char *filename);

int main (int argc, char **argv)
{
	int	i;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s filename [more filenames...]\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}

	for (i=1; i<argc; i++)
		print_power(argv[i]);

	return 0;
}

void print_power (char *filename)
{
	AFfilehandle	file;
	double		*sums, *frames;
	int		channelCount, windowSize, frameCount;
	int		i, c;
	struct smooth	*powsmooth;
	int		winStart, winEnd;
	int		lastWindow = FALSE;
	double		pow, maxpow;

	double		level, peak, minSample = 1, maxSample = -1;

	file = afOpenFile(filename, "r", NULL);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "Could not open file %s.\n", filename);
		return;
	}

	channelCount = afGetChannels(file, AF_DEFAULT_TRACK);
	windowSize = afGetRate(file, AF_DEFAULT_TRACK) / 100;
	frameCount = afGetFrameCount(file, AF_DEFAULT_TRACK);

	sums = calloc(channelCount, sizeof (double));
	for (c=0; c<channelCount; c++)
		sums[c] = 0;

	frames = calloc(channelCount * windowSize, sizeof (double));

	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_DOUBLE,
		sizeof (double));

	powsmooth = calloc(channelCount, sizeof (struct smooth));
	for (c=0; c<channelCount; c++)
	{
		/* Use a 100-element (1 second) window. */
		powsmooth[c].length = 100;
		powsmooth[c].buf = calloc(powsmooth[c].length, sizeof (double));
		powsmooth[c].start = 0;
		powsmooth[c].n = 0;
	}

	winStart = 0;
	winEnd = 0;
	lastWindow = FALSE;
	maxpow = 0;

	do
	{
		winEnd = winStart + windowSize;

		if (winEnd >= frameCount)
		{
			winEnd = frameCount;
			lastWindow = TRUE;
		}

		afReadFrames(file, AF_DEFAULT_TRACK, frames, windowSize);

		for (c=0; c<channelCount; c++)
		{
			sums[c] = 0;

			for (i=0; i < winEnd - winStart; i++)
			{
				double	sample;

				sample = frames[i*channelCount + c];
				sums[c] += sample*sample;

				if (sample > maxSample)
					maxSample = sample;
				if (sample < minSample)
					minSample = sample;
			}
		}

		/* Compute power for each channel. */
		for (c=0; c<channelCount; c++)
		{
			double pow;
			int end;

			pow = sums[c] / (winEnd - winStart);

			end = (powsmooth[c].start + powsmooth[c].n) %
				powsmooth[c].length;
			powsmooth[c].buf[end] = pow;

			if (powsmooth[c].n == powsmooth[c].length)
			{
				powsmooth[c].start = (powsmooth[c].start + 1) % powsmooth[c].length;
				pow = get_smoothed_data(&powsmooth[c]);
				if (pow > maxpow)
					maxpow = pow;
			}
			else
			{
				powsmooth[c].n++;
			}
		}

		winStart += windowSize;
	} while (!lastWindow);

	for (c = 0; c < channelCount; c++)
	{
		pow = get_smoothed_data(&powsmooth[c]);
		if (pow > maxpow)
			maxpow = pow;
	}

	free(sums);
	free(frames);
	for (c=0; c<channelCount; c++)
		free(powsmooth[c].buf);
	free(powsmooth);

	level = sqrt(maxpow);

	afCloseFile(file);

	printf("file: %s\n", filename);

	printf("level (dB): %f\n", 20 * log10(level));
	printf("peak-: %f\n", minSample);
	printf("peak+: %f\n", maxSample);

	peak = fabs(minSample);
	if (peak < fabs(maxSample))
		peak = fabs(maxSample);

	printf("peak (dB): %f\n", 20 * log10(peak));
}
