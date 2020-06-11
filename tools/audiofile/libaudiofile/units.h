/*
	Audio File Library
	Copyright (C) 2000, Michael Pruett <michael@68k.org>

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
	units.h

	This file defines the internal Unit and CompressionUnit
	structures for the Audio File Library.
*/

#ifndef UNIT_H
#define UNIT_H

#include "audiofile.h"
#include "afinternal.h"

struct AudioFormat;
class FileModule;

struct Unit
{
	int	fileFormat;	/* AF_FILEFMT_... */
	const char *name;		/* a 2-3 word name of the file format */
	const char *description;	/* a more descriptive name for the format */
	const char *label;		/* a 4-character label for the format */
	bool implemented;	/* if implemented */

	AFfilesetup (*completesetup) (AFfilesetup setup);
	bool (*recognize) (File *fh);

	int defaultSampleFormat;
	int defaultSampleWidth;

	int compressionTypeCount;
	const int *compressionTypes;

	int markerCount;

	int instrumentCount;
	int loopPerInstrumentCount;

	int instrumentParameterCount;
	const InstParamInfo *instrumentParameters;
};

struct CompressionUnit
{
	int	compressionID;	/* AF_COMPRESSION_... */
	bool implemented;
	const char *label;		/* 4-character (approximately) label */
	const char *shortname;	/* short name in English */
	const char *name;		/* long name in English */
	double squishFactor;	/* compression ratio */
	int	nativeSampleFormat;	/* AF_SAMPFMT_... */
	int	nativeSampleWidth;	/* sample width in bits */
	bool	needsRebuffer;	/* if there are chunk boundary requirements */
	bool	multiple_of;	/* can accept any multiple of chunksize */
	bool	(*fmtok) (AudioFormat *format);

	FileModule *(*initcompress) (Track *track, File *fh,
		bool seekok, bool headerless, AFframecount *chunkframes);
	FileModule *(*initdecompress) (Track *track, File *fh,
		bool seekok, bool headerless, AFframecount *chunkframes);
};

#define _AF_NUM_UNITS 17
#define _AF_NUM_COMPRESSION 7

extern const Unit _af_units[_AF_NUM_UNITS];
extern const CompressionUnit _af_compression[_AF_NUM_COMPRESSION];

#endif /* UNIT_H */
