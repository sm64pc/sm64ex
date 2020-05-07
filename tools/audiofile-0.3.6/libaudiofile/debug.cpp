/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>
	Copyright (C) 2000, Silicon Graphics, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA  02110-1301  USA
*/

/*
	debug.cpp

	This file contains debugging routines for the Audio File
	Library.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "audiofile.h"
#include "aupvlist.h"

#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "aupvinternal.h"
#include "byteorder.h"
#include "compression.h"
#include "debug.h"
#include "units.h"
#include "util.h"

void _af_print_pvlist (AUpvlist list)
{
	assert(list);

	printf("list.valid: %d\n", list->valid);
	printf("list.count: %zu\n", list->count);

	for (unsigned i=0; i<list->count; i++)
	{
		printf("item %u valid %d, should be %d\n",
			i, list->items[i].valid, _AU_VALID_PVITEM);

		switch (list->items[i].type)
		{
			case AU_PVTYPE_LONG:
				printf("item #%u, parameter %d, long: %ld\n",
					i, list->items[i].parameter,
					list->items[i].value.l);
				break;
			case AU_PVTYPE_DOUBLE:
				printf("item #%u, parameter %d, double: %f\n",
					i, list->items[i].parameter,
					list->items[i].value.d);
				break;
			case AU_PVTYPE_PTR:
				printf("item #%u, parameter %d, pointer: %p\n",
					i, list->items[i].parameter,
					list->items[i].value.v);
				break;

			default:
				printf("item #%u, invalid type %d\n", i,
					list->items[i].type);
				assert(false);
				break;
		}
	}
}

void _af_print_audioformat (AudioFormat *fmt)
{
	/* sampleRate, channelCount */
	printf("{ %7.2f Hz %d ch ", fmt->sampleRate, fmt->channelCount);

	/* sampleFormat, sampleWidth */
	switch (fmt->sampleFormat)
	{
		case AF_SAMPFMT_TWOSCOMP:
			printf("%db 2 ", fmt->sampleWidth);
			break;
		case AF_SAMPFMT_UNSIGNED:
			printf("%db u ", fmt->sampleWidth);
			break;
		case AF_SAMPFMT_FLOAT:
			printf("flt ");
			break;
		case AF_SAMPFMT_DOUBLE:
			printf("dbl ");
			break;
		default:
			printf("%dsampfmt? ", fmt->sampleFormat);
	}

	/* pcm */
	printf("(%.30g+-%.30g [%.30g,%.30g]) ",
		fmt->pcm.intercept, fmt->pcm.slope,
		fmt->pcm.minClip, fmt->pcm.maxClip);

	/* byteOrder */
	switch (fmt->byteOrder)
	{
		case AF_BYTEORDER_BIGENDIAN:
			printf("big ");
			break;
		case AF_BYTEORDER_LITTLEENDIAN:
			printf("little ");
			break;
		default:
			printf("%dbyteorder? ", fmt->byteOrder);
			break;
	}

	/* compression */
	{
		const CompressionUnit *unit = _af_compression_unit_from_id(fmt->compressionType);
		if (!unit)
			printf("%dcompression?", fmt->compressionType);
		else if (fmt->compressionType == AF_COMPRESSION_NONE)
			printf("pcm");
		else
			printf("%s", unit->label);
	}

	printf(" }");
}

void _af_print_tracks (AFfilehandle filehandle)
{
	for (int i=0; i<filehandle->m_trackCount; i++)
	{
		Track *track = &filehandle->m_tracks[i];
		printf("track %d\n", i);
		printf(" id %d\n", track->id);
		printf(" sample format\n");
		_af_print_audioformat(&track->f);
		printf(" virtual format\n");
		_af_print_audioformat(&track->v);
		printf(" total file frames: %jd\n",
			(intmax_t) track->totalfframes);
		printf(" total virtual frames: %jd\n",
			(intmax_t) track->totalvframes);
		printf(" next file frame: %jd\n",
			(intmax_t) track->nextfframe);
		printf(" next virtual frame: %jd\n",
			(intmax_t) track->nextvframe);
		printf(" frames to ignore: %jd\n",
			(intmax_t) track->frames2ignore);

		printf(" data_size: %jd\n",
			(intmax_t) track->data_size);
		printf(" fpos_first_frame: %jd\n",
			(intmax_t) track->fpos_first_frame);
		printf(" fpos_next_frame: %jd\n",
			(intmax_t) track->fpos_next_frame);
		printf(" fpos_after_data: %jd\n",
			(intmax_t) track->fpos_after_data);

		printf(" channel matrix:");
		_af_print_channel_matrix(track->channelMatrix,
			track->f.channelCount, track->v.channelCount);
		printf("\n");

		printf(" marker count: %d\n", track->markerCount);
	}
}

void _af_print_filehandle (AFfilehandle filehandle)
{
	printf("file handle: 0x%p\n", filehandle);

	if (filehandle->m_valid == _AF_VALID_FILEHANDLE)
		printf("valid\n");
	else
		printf("invalid!\n");

	printf(" access: ");
	if (filehandle->m_access == _AF_READ_ACCESS)
		putchar('r');
	else
		putchar('w');

	printf(" fileFormat: %d\n", filehandle->m_fileFormat);

	printf(" instrument count: %d\n", filehandle->m_instrumentCount);
	printf(" instruments: 0x%p\n", filehandle->m_instruments);

	printf(" miscellaneous count: %d\n", filehandle->m_miscellaneousCount);
	printf(" miscellaneous: 0x%p\n", filehandle->m_miscellaneous);

	printf(" trackCount: %d\n", filehandle->m_trackCount);
	printf(" tracks: 0x%p\n", filehandle->m_tracks);
	_af_print_tracks(filehandle);
}

void _af_print_channel_matrix (double *matrix, int fchans, int vchans)
{
	int v, f;

	if (!matrix)
	{
		printf("NULL");
		return;
	}

	printf("{");
	for (v=0; v < vchans; v++)
	{
		if (v) printf(" ");
		printf("{");
		for (f=0; f < fchans; f++)
		{
			if (f) printf(" ");
			printf("%5.2f", *(matrix + v*fchans + f));
		}
		printf("}");
	}
	printf("}");
}

void _af_print_frame (AFframecount frameno, double *frame, int nchannels,
	char *formatstring, int numberwidth,
	double slope, double intercept, double minclip, double maxclip)
{
	char linebuf[81];
	int wavewidth = 78 - numberwidth*nchannels - 6;
	int c;

	memset(linebuf, ' ', 80);
	linebuf[0] = '|';
	linebuf[wavewidth-1] = '|';
	linebuf[wavewidth] = 0;

	printf("%05jd ", (intmax_t) frameno);

	for (c=0; c < nchannels; c++)
	{
		double pcm = frame[c];
		printf(formatstring, pcm);
	}
	for (c=0; c < nchannels; c++)
	{
		double pcm = frame[c], volts;
		if (maxclip > minclip)
		{
			if (pcm < minclip) pcm = minclip;
			if (pcm > maxclip) pcm = maxclip;
		}
		volts = (pcm - intercept) / slope;
		linebuf[(int)((volts/2 + 0.5)*(wavewidth-3)) + 1] = '0' + c;
	}
	printf("%s\n", linebuf);
}
