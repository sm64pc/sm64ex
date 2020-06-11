/*
	Audio File Library
	Copyright (C) 1999-2000, Michael Pruett <michael@68k.org>
	Copyright (C) 2000-2001, Silicon Graphics, Inc.

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
	pcm.cpp

	This file declares default PCM mappings and defines routines
	for accessing and modifying PCM mappings in a track.
*/

#include "config.h"

#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "modules/Module.h"
#include "modules/ModuleState.h"
#include "pcm.h"
#include "util.h"

extern const PCMInfo _af_default_signed_integer_pcm_mappings[] =
{
	{0, 0, 0, 0},
	{SLOPE_INT8, 0, MIN_INT8, MAX_INT8},
	{SLOPE_INT16, 0, MIN_INT16, MAX_INT16},
	{SLOPE_INT24, 0, MIN_INT24, MAX_INT24},
	{SLOPE_INT32, 0, MIN_INT32, MAX_INT32}
};

extern const PCMInfo _af_default_unsigned_integer_pcm_mappings[] =
{
	{0, 0, 0, 0},
	{SLOPE_INT8, INTERCEPT_U_INT8, 0, MAX_U_INT8},
	{SLOPE_INT16, INTERCEPT_U_INT16, 0, MAX_U_INT16},
	{SLOPE_INT24, INTERCEPT_U_INT24, 0, MAX_U_INT24},
	{SLOPE_INT32, INTERCEPT_U_INT32, 0, MAX_U_INT32}
};

extern const PCMInfo _af_default_float_pcm_mapping =
{1, 0, 0, 0};

extern const PCMInfo _af_default_double_pcm_mapping =
{1, 0, 0, 0};

/*
	Initialize the PCM mapping for a given track.
*/
void afInitPCMMapping (AFfilesetup setup, int trackid,
	double slope, double intercept, double minClip, double maxClip)
{
	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	track->f.pcm.slope = slope;
	track->f.pcm.intercept = intercept;
	track->f.pcm.minClip = minClip;
	track->f.pcm.maxClip = maxClip;
}

int afSetVirtualPCMMapping (AFfilehandle file, int trackid,
	double slope, double intercept, double minClip, double maxClip)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	track->v.pcm.slope = slope;
	track->v.pcm.intercept = intercept;
	track->v.pcm.minClip = minClip;
	track->v.pcm.maxClip = maxClip;

	track->ms->setDirty();

	return 0;
}

int afSetTrackPCMMapping (AFfilehandle file, int trackid,
	double slope, double intercept, double minClip, double maxClip)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	/*
		NOTE: this is highly unusual: we don't ordinarily
		change track.f after the file is opened.

		PCM mapping is the exception because this information
		is not encoded in sound files' headers using today's
		formats, so the user will probably want to set this
		information on a regular basis.  The defaults may or
		may not be what the user wants.
	*/

	track->f.pcm.slope = slope;
	track->f.pcm.intercept = intercept;
	track->f.pcm.minClip = minClip;
	track->f.pcm.maxClip = maxClip;

	track->ms->setDirty();

	return 0;
}

void afGetPCMMapping (AFfilehandle file, int trackid,
	double *slope, double *intercept, double *minClip, double *maxClip)
{
	if (!_af_filehandle_ok(file))
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	if (slope)
		*slope = track->f.pcm.slope;
	if (intercept)
		*intercept = track->f.pcm.intercept;
	if (minClip)
		*minClip = track->f.pcm.minClip;
	if (maxClip)
		*maxClip = track->f.pcm.maxClip;
}

void afGetVirtualPCMMapping (AFfilehandle file, int trackid,
	double *slope, double *intercept, double *minClip, double *maxClip)
{
	if (!_af_filehandle_ok(file))
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	if (slope)
		*slope = track->v.pcm.slope;
	if (intercept)
		*intercept = track->v.pcm.intercept;
	if (minClip)
		*minClip = track->v.pcm.minClip;
	if (maxClip)
		*maxClip = track->v.pcm.maxClip;
}
