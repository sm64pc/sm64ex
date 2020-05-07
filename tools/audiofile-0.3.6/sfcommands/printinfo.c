/*
	Audio File Library

	Copyright 1998, 2011, Michael Pruett <michael@68k.org>

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
	printinfo.c

	This file contains the function used by the sf commands to print
	information regarding a file.
*/

#include "config.h"
#include "printinfo.h"

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include <audiofile.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static char *copyrightstring (AFfilehandle file);

bool printfileinfo (const char *filename)
{
	AFfilehandle file = afOpenFile(filename, "r", NULL);
	if (!file)
		return false;

	int fileFormat = afGetFileFormat(file, NULL);
	const char *formatstring =
		(const char *) afQueryPointer(AF_QUERYTYPE_FILEFMT, AF_QUERY_DESC,
			fileFormat, 0, 0);
	const char *labelstring =
		(const char *) afQueryPointer(AF_QUERYTYPE_FILEFMT, AF_QUERY_LABEL,
			fileFormat, 0, 0);

	if (!formatstring || !labelstring)
		return false;

	printf("File Name      %s\n", filename);
	printf("File Format    %s (%s)\n", formatstring, labelstring);

	int sampleFormat, sampleWidth;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);

	int byteOrder = afGetByteOrder(file, AF_DEFAULT_TRACK);

	printf("Data Format    ");

	int compressionType = afGetCompression(file, AF_DEFAULT_TRACK);
	if (compressionType == AF_COMPRESSION_NONE)
	{
		switch (sampleFormat)
		{
			case AF_SAMPFMT_TWOSCOMP:
				printf("%d-bit integer (2's complement, %s)",
					sampleWidth,
					byteOrder == AF_BYTEORDER_BIGENDIAN ?
						"big endian" : "little endian");
				break;
			case AF_SAMPFMT_UNSIGNED:
				printf("%d-bit integer (unsigned, %s)",
					sampleWidth,
					byteOrder == AF_BYTEORDER_BIGENDIAN ?
						"big endian" : "little endian");
				break;
			case AF_SAMPFMT_FLOAT:
				printf("single-precision (32-bit) floating point, %s",
					byteOrder == AF_BYTEORDER_BIGENDIAN ?
						"big endian" : "little endian");
				break;
			case AF_SAMPFMT_DOUBLE:
				printf("double-precision (64-bit) floating point, %s",
					byteOrder == AF_BYTEORDER_BIGENDIAN ?
						"big endian" : "little endian");
				break;
			default:
				printf("unknown");
				break;
		}
	}
	else
	{
		const char *compressionName =
			(const char *) afQueryPointer(AF_QUERYTYPE_COMPRESSION,
				AF_QUERY_NAME, compressionType, 0, 0);

		if (!compressionName)
			printf("unknown compression");
		else
			printf("%s compression", compressionName);
	}
	printf("\n");

	printf("Audio Data     %jd bytes begins at offset %jd (%jx hex)\n",
		(intmax_t) afGetTrackBytes(file, AF_DEFAULT_TRACK),
		(intmax_t) afGetDataOffset(file, AF_DEFAULT_TRACK),
		(uintmax_t) afGetDataOffset(file, AF_DEFAULT_TRACK));

	printf("               %d channel%s, %jd frames\n",
		afGetChannels(file, AF_DEFAULT_TRACK),
		afGetChannels(file, AF_DEFAULT_TRACK) > 1 ? "s" : "",
		(intmax_t) afGetFrameCount(file, AF_DEFAULT_TRACK));

	printf("Sampling Rate  %.2f Hz\n", afGetRate(file, AF_DEFAULT_TRACK));

	printf("Duration       %.3f seconds\n",
		afGetFrameCount(file, AF_DEFAULT_TRACK) /
		afGetRate(file, AF_DEFAULT_TRACK));

	char *copyright = copyrightstring(file);
	if (copyright)
	{
		printf("Copyright      %s\n", copyright);
		free(copyright);
	}

	afCloseFile(file);

	return true;
}

static char *copyrightstring (AFfilehandle file)
{
	char	*copyright = NULL;
	int		*miscids;
	int		i, misccount;

	misccount = afGetMiscIDs(file, NULL);
	miscids = (int *) malloc(sizeof (int) * misccount);
	afGetMiscIDs(file, miscids);

	for (i=0; i<misccount; i++)
	{
		if (afGetMiscType(file, miscids[i]) != AF_MISC_COPY)
			continue;

		/*
			If this code executes, the miscellaneous chunk is a
			copyright chunk.
		*/
		int datasize = afGetMiscSize(file, miscids[i]);
		char *data = (char *) malloc(datasize);
		afReadMisc(file, miscids[i], data, datasize);
		copyright = data;
		break;
	}

	free(miscids);

	return copyright;
}

/*
    0.73s 44100.00 aiff  1ch 16b -- flute.aif
*/
bool printshortinfo (const char *filename)
{
	AFfilehandle file = afOpenFile(filename, "r", NULL);;
	if (!file)
		return false;

	int fileFormat = afGetFileFormat(file, NULL);
	double sampleRate = afGetRate(file, AF_DEFAULT_TRACK);
	double duration = afGetFrameCount(file, AF_DEFAULT_TRACK) / sampleRate;
	const char *labelString =
		(const char *) afQueryPointer(AF_QUERYTYPE_FILEFMT, AF_QUERY_LABEL,
			fileFormat, 0, 0);
	int channels = afGetChannels(file, AF_DEFAULT_TRACK);
	int sampleFormat, sampleWidth;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);

	int compressionType = afGetCompression(file, AF_DEFAULT_TRACK);
	const char *compressionName = "--";
	if (compressionType != AF_COMPRESSION_NONE)
	{
		compressionName =
			(const char *) afQueryPointer(AF_QUERYTYPE_COMPRESSION,
				AF_QUERY_NAME, compressionType, 0, 0);
		if (!compressionName)
			compressionName = "unk";
	}

	printf("%8.2fs %8.2f %4s %2dch %2db %s %s\n",
		duration,
		sampleRate,
		labelString,
		channels,
		sampleWidth,
		compressionName,
		filename);

	afCloseFile(file);

	return true;
}
