/*
	Audio File Library
	Copyright (C) 1998-1999, Michael Pruett <michael@68k.org>
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
	aes.c

	This file contains routines for dealing with AES recording data.
*/

#include "config.h"

#include <string.h>
#include <assert.h>

#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "util.h"

void afInitAESChannelData (AFfilesetup setup, int trackid)
{
	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	track->aesDataSet = true;
}

void afInitAESChannelDataTo (AFfilesetup setup, int trackid, int willBeData)
{
	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	track->aesDataSet = willBeData;
}

int afGetAESChannelData (AFfilehandle file, int trackid, unsigned char buf[24])
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	if (!track->hasAESData)
	{
		if (buf)
			memset(buf, 0, 24);
		return 0;
	}

	if (buf)
		memcpy(buf, track->aesData, 24);

	return 1;
}

void afSetAESChannelData (AFfilehandle file, int trackid, unsigned char buf[24])
{
	if (!_af_filehandle_ok(file))
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	if (!file->checkCanWrite())
		return;

	if (track->hasAESData)
	{
		memcpy(track->aesData, buf, 24);
	}
	else
	{
		_af_error(AF_BAD_NOAESDATA,
			"unable to store AES channel status data for track %d",
			trackid);
	}
}
