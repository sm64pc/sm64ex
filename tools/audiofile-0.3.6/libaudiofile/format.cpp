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
	audiofile.c

	This file implements many of the main interface routines of the
	Audio File Library.
*/

#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "modules/Module.h"
#include "modules/ModuleState.h"
#include "units.h"
#include "util.h"

AFfileoffset afGetDataOffset (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->fpos_first_frame;
}

AFfileoffset afGetTrackBytes (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->data_size;
}

/*
	afGetFrameSize returns the size (in bytes) of a sample frame from
	the specified track of an audio file.

	stretch3to4 == true: size which user sees
	stretch3to4 == false: size used in file
*/
float afGetFrameSize (AFfilehandle file, int trackid, int stretch3to4)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return _af_format_frame_size(&track->f, stretch3to4);
}

float afGetVirtualFrameSize (AFfilehandle file, int trackid, int stretch3to4)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return _af_format_frame_size(&track->v, stretch3to4);
}

AFframecount afSeekFrame (AFfilehandle file, int trackid, AFframecount frame)
{
	if (!_af_filehandle_ok(file))
		return -1;

	if (!file->checkCanRead())
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	if (track->ms->isDirty() && track->ms->setup(file, track) == AF_FAIL)
		return -1;

	if (frame < 0)
		return track->nextvframe;

	/* Optimize the case of seeking to the current position. */
	if (frame == track->nextvframe)
		return track->nextvframe;

	/* Limit request to the number of frames in the file. */
	if (track->totalvframes != -1)
		if (frame > track->totalvframes)
			frame = track->totalvframes - 1;

	/*
		Now that the modules are not dirty and frame
		represents a valid virtual frame, we call
		_AFsetupmodules again after setting track->nextvframe.

		_AFsetupmodules will look at track->nextvframe and
		compute track->nextfframe in clever and mysterious
		ways.
	*/
	track->nextvframe = frame;

	if (track->ms->setup(file, track) == AF_FAIL)
		return -1;

	return track->nextvframe;
}

AFfileoffset afTellFrame (AFfilehandle file, int trackid)
{
	return afSeekFrame(file, trackid, -1);
}

int afSetVirtualByteOrder (AFfilehandle file, int trackid, int byteorder)
{
	if (!_af_filehandle_ok(file))
		return AF_FAIL;

	Track *track = file->getTrack(trackid);
	if (!track)
		return AF_FAIL;

	if (byteorder != AF_BYTEORDER_BIGENDIAN &&
		byteorder != AF_BYTEORDER_LITTLEENDIAN)
	{
		_af_error(AF_BAD_BYTEORDER, "invalid byte order %d", byteorder);
		return AF_FAIL;
	}

	track->v.byteOrder = byteorder;
	track->ms->setDirty();

	return AF_SUCCEED;
}

int afGetByteOrder (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->f.byteOrder;
}

int afGetVirtualByteOrder (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->v.byteOrder;
}

AFframecount afGetFrameCount (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	if (track->ms->isDirty() && track->ms->setup(file, track) == AF_FAIL)
		return -1;

	return track->totalvframes;
}

double afGetRate (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->f.sampleRate;
}

int afGetChannels (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->f.channelCount;
}

void afGetSampleFormat (AFfilehandle file, int trackid, int *sampleFormat, int *sampleWidth)
{
	if (!_af_filehandle_ok(file))
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	if (sampleFormat)
		*sampleFormat = track->f.sampleFormat;

	if (sampleWidth)
		*sampleWidth = track->f.sampleWidth;
}

void afGetVirtualSampleFormat (AFfilehandle file, int trackid, int *sampleFormat, int *sampleWidth)
{
	if (!_af_filehandle_ok(file))
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	if (sampleFormat)
		*sampleFormat = track->v.sampleFormat;

	if (sampleWidth)
		*sampleWidth = track->v.sampleWidth;
}

int afSetVirtualSampleFormat (AFfilehandle file, int trackid,
	int sampleFormat, int sampleWidth)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	if (_af_set_sample_format(&track->v, sampleFormat, sampleWidth) == AF_FAIL)
		return -1;

	track->ms->setDirty();

	return 0;
}

/* XXXmpruett fix the version */
int afGetFileFormat (AFfilehandle file, int *version)
{
	if (!_af_filehandle_ok(file))
		return -1;

	if (version != NULL)
		*version = file->getVersion();

	return file->m_fileFormat;
}

int afSetVirtualChannels (AFfilehandle file, int trackid, int channelCount)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	track->v.channelCount = channelCount;
	track->ms->setDirty();

	if (track->channelMatrix)
		free(track->channelMatrix);
	track->channelMatrix = NULL;

	return 0;
}

double afGetVirtualRate (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->v.sampleRate;
}

int afSetVirtualRate (AFfilehandle file, int trackid, double rate)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	if (rate < 0)
	{
		_af_error(AF_BAD_RATE, "invalid sampling rate %.30g", rate);
		return -1;
	}

	track->v.sampleRate = rate;
	track->ms->setDirty();

	return 0;
}

void afSetChannelMatrix (AFfilehandle file, int trackid, double* matrix)
{
	if (!_af_filehandle_ok(file))
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	if (track->channelMatrix != NULL)
		free(track->channelMatrix);
	track->channelMatrix = NULL;

	if (matrix != NULL)
	{
		int	i, size;

		size = track->v.channelCount * track->f.channelCount;

		track->channelMatrix = (double *) malloc(size * sizeof (double));

		for (i = 0; i < size; i++)
			track->channelMatrix[i] = matrix[i];
	}
}

int afGetVirtualChannels (AFfilehandle file, int trackid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	return track->v.channelCount;
}
