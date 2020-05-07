/*
	Audio File Library
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
	Raw.cpp

	This file contains code for reading and writing raw audio
	data files.
*/

#include "config.h"
#include "Raw.h"

#include "File.h"
#include "Setup.h"
#include "Track.h"
#include "util.h"

static const _AFfilesetup rawDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	// valid
	AF_FILE_RAWDATA,		// fileFormat
	true,	// trackSet
	true,	// instrumentSet
	true,	// miscellaneousSet
	1,		// trackCount
	NULL,	// tracks
	0,		// instrumentCount
	NULL,	// instruments
	0,		// miscellaneousCount
	NULL	// miscellaneous
};

const int _af_raw_compression_types[_AF_RAW_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW
};

bool RawFile::recognize(File *fh)
{
	return false;
}

status RawFile::readInit(AFfilesetup filesetup)
{
	if (!filesetup)
	{
		_af_error(AF_BAD_FILESETUP, "a valid AFfilesetup is required for reading raw data");
		return AF_FAIL;
	}

	if (initFromSetup(filesetup) == AF_FAIL)
		return AF_FAIL;

	Track *track = getTrack();

	/* Set the track's data offset. */
	if (filesetup->tracks[0].dataOffsetSet)
		track->fpos_first_frame = filesetup->tracks[0].dataOffset;
	else
		track->fpos_first_frame = 0;

	/* Set the track's frame count. */
	if (filesetup->tracks[0].frameCountSet)
	{
		track->totalfframes = filesetup->tracks[0].frameCount;
	}
	else
	{
		AFfileoffset filesize = m_fh->length();
		if (filesize == -1)
			track->totalfframes = -1;
		else
		{
			/* Ensure that the data offset is valid. */
			if (track->fpos_first_frame > filesize)
			{
				_af_error(AF_BAD_FILESETUP, "data offset is larger than file size");
				return AF_FAIL;
			}

			filesize -= track->fpos_first_frame;
			track->totalfframes = filesize / (int) _af_format_frame_size(&track->f, false);
		}
		track->data_size = filesize;
	}

	return AF_SUCCEED;
}

status RawFile::writeInit(AFfilesetup filesetup)
{
	if (initFromSetup(filesetup) == AF_FAIL)
		return AF_FAIL;

	Track *track = getTrack();

	track->totalfframes = 0;
	if (filesetup->tracks[0].dataOffsetSet)
		track->fpos_first_frame = filesetup->tracks[0].dataOffset;
	else
		track->fpos_first_frame = 0;

	return AF_SUCCEED;
}

status RawFile::update()
{
	return AF_SUCCEED;
}

AFfilesetup RawFile::completeSetup(AFfilesetup setup)
{
	AFfilesetup	newSetup;

	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_FILESETUP, "raw file must have exactly one track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
	{
		_af_error(AF_BAD_FILESETUP, "could not access track in file setup");
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "raw file cannot have AES data");
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount != 0)
	{
		_af_error(AF_BAD_NUMMARKS, "raw file cannot have markers");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount != 0)
	{
		_af_error(AF_BAD_NUMINSTS, "raw file cannot have instruments");
		return AF_NULL_FILESETUP;
	}

	if (setup->miscellaneousSet && setup->miscellaneousCount != 0)
	{
		_af_error(AF_BAD_NUMMISC, "raw file cannot have miscellaneous data");
		return AF_NULL_FILESETUP;
	}

	newSetup = (_AFfilesetup *) _af_malloc(sizeof (_AFfilesetup));
	*newSetup = rawDefaultFileSetup;

	newSetup->tracks = (TrackSetup *) _af_malloc(sizeof (TrackSetup));
	newSetup->tracks[0] = setup->tracks[0];
	newSetup->tracks[0].f.compressionParams = NULL;

	newSetup->tracks[0].markerCount = 0;
	newSetup->tracks[0].markers = NULL;

	return newSetup;
}
