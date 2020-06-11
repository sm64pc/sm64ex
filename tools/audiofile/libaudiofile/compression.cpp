/*
	Audio File Library
	Copyright (C) 1999-2000, Michael Pruett <michael@68k.org>
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
	compression.cpp

	This file contains routines for configuring compressed audio.
*/

#include "config.h"

#include <assert.h>

#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "audiofile.h"
#include "aupvlist.h"
#include "units.h"
#include "util.h"

const CompressionUnit *_af_compression_unit_from_id (int compressionid)
{
	for (int i=0; i<_AF_NUM_COMPRESSION; i++)
		if (_af_compression[i].compressionID == compressionid)
			return &_af_compression[i];

	_af_error(AF_BAD_COMPTYPE, "compression type %d not available", compressionid);
	return NULL;
}

int afGetCompression (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->f.compressionType;
}

void afInitCompression (AFfilesetup setup, int trackid, int compression)
{
	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	if (!_af_compression_unit_from_id(compression))
		return;

	track->compressionSet = true;
	track->f.compressionType = compression;
}

#if 0
int afGetCompressionParams (AFfilehandle file, int trackid,
	int *compression, AUpvlist pvlist, int numitems)
{
	assert(file);
	assert(trackid == AF_DEFAULT_TRACK);
}

void afInitCompressionParams (AFfilesetup setup, int trackid,
	int compression, AUpvlist pvlist, int numitems)
{
	assert(setup);
	assert(trackid == AF_DEFAULT_TRACK);
}
#endif
