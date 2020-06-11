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
	Loop.cpp

	All routines that operate on loops.
*/

#include "config.h"

#include "FileHandle.h"
#include "Instrument.h"
#include "Setup.h"
#include "afinternal.h"
#include "audiofile.h"
#include "util.h"

void afInitLoopIDs (AFfilesetup setup, int instid, const int *loopids, int nloops)
{
	if (!_af_filesetup_ok(setup))
		return;

	if (!_af_unique_ids(loopids, nloops, "loop", AF_BAD_LOOPID))
		return;

	InstrumentSetup *instrument = setup->getInstrument(instid);
	if (!instrument)
		return;

	instrument->freeLoops();
	if (!instrument->allocateLoops(nloops))
		return;

	for (int i=0; i < nloops; i++)
		instrument->loops[i].id = loopids[i];
}

int afGetLoopIDs (AFfilehandle file, int instid, int *loopids)
{
	if (!_af_filehandle_ok(file))
		return AF_FAIL;

	Instrument *instrument = file->getInstrument(instid);
	if (!instrument)
		return AF_FAIL;

	if (loopids)
		for (int i=0; i < instrument->loopCount; i++)
			loopids[i] = instrument->loops[i].id;

	return instrument->loopCount;
}

/*
	getLoop returns pointer to requested loop if it exists, and if
	mustWrite is true, only if handle is writable.
*/

static Loop *getLoop (AFfilehandle handle, int instid, int loopid,
	bool mustWrite)
{
	if (!_af_filehandle_ok(handle))
		return NULL;

	if (mustWrite && !handle->checkCanWrite())
		return NULL;

	Instrument *instrument = handle->getInstrument(instid);
	if (!instrument)
		return NULL;

	return instrument->getLoop(loopid);
}

/*
	Set loop mode (as in AF_LOOP_MODE_...).
*/
void afSetLoopMode (AFfilehandle file, int instid, int loopid, int mode)
{
	Loop *loop = getLoop(file, instid, loopid, true);

	if (!loop)
		return;

	if (mode != AF_LOOP_MODE_NOLOOP &&
		mode != AF_LOOP_MODE_FORW &&
		mode != AF_LOOP_MODE_FORWBAKW)
	{
		_af_error(AF_BAD_LOOPMODE, "unrecognized loop mode %d", mode);
		return;
	}

	loop->mode = mode;
}

/*
	Get loop mode (as in AF_LOOP_MODE_...).
*/
int afGetLoopMode (AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);

	if (!loop)
		return -1;

	return loop->mode;
}

/*
	Set loop count.
*/
int afSetLoopCount (AFfilehandle file, int instid, int loopid, int count)
{
	Loop *loop = getLoop(file, instid, loopid, true);

	if (!loop)
		return AF_FAIL;

	if (count < 1)
	{
		_af_error(AF_BAD_LOOPCOUNT, "invalid loop count: %d", count);
		return AF_FAIL;
	}

	loop->count = count;
	return AF_SUCCEED;
}

/*
	Get loop count.
*/
int afGetLoopCount(AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);

	if (!loop)
		return -1;

	return loop->count;
}

/*
	Set loop start marker id in the file structure
*/
void afSetLoopStart(AFfilehandle file, int instid, int loopid, int markid)
{
	Loop *loop = getLoop(file, instid, loopid, true);

	if (!loop)
		return;

	loop->beginMarker = markid;
}

/*
	Get loop start marker id.
*/
int afGetLoopStart (AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);

	if (!loop)
		return -1;

	return loop->beginMarker;
}

/*
	Set loop start frame in the file structure.
*/
int afSetLoopStartFrame (AFfilehandle file, int instid, int loopid, AFframecount startFrame)
{
	Loop *loop = getLoop(file, instid, loopid, true);

	if (!loop)
		return -1;

	if (startFrame < 0)
	{
		_af_error(AF_BAD_FRAME, "loop start frame must not be negative");
		return AF_FAIL;
	}

	int	trackid = loop->trackid;
	int beginMarker = loop->beginMarker;

	afSetMarkPosition(file, trackid, beginMarker, startFrame);
	return AF_SUCCEED;
}

/*
	Get loop start frame.
*/
AFframecount afGetLoopStartFrame (AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);
	if (!loop)
		return -1;

	int trackid = loop->trackid;
	int beginMarker = loop->beginMarker;

	return afGetMarkPosition(file, trackid, beginMarker);
}

/*
	Set loop track id.
*/
void afSetLoopTrack (AFfilehandle file, int instid, int loopid, int track)
{
	Loop *loop = getLoop(file, instid, loopid, true);

	if (!loop) return;

	loop->trackid = track;
}

/*
	Get loop track.
*/
int afGetLoopTrack (AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);

	if (!loop)
		return -1;

	return loop->trackid;
}

/*
	Set loop end frame marker id.
*/
void afSetLoopEnd (AFfilehandle file, int instid, int loopid, int markid)
{
	Loop *loop = getLoop(file, instid, loopid, true);

	if (!loop)
		return;

	loop->endMarker = markid;
}

/*
	Get loop end frame marker id.
*/
int afGetLoopEnd (AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);

	if (!loop)
		return -1;

	return loop->endMarker;
}

/*
	Set loop end frame.
*/
int afSetLoopEndFrame (AFfilehandle file, int instid, int loopid, AFframecount endFrame)
{
	Loop *loop = getLoop(file, instid, loopid, true);
	if (!loop)
		return -1;

	if (endFrame < 0)
	{
		_af_error(AF_BAD_FRAME, "loop end frame must not be negative");
		return AF_FAIL;
	}

	int trackid = loop->trackid;
	int endMarker = loop->endMarker;

	afSetMarkPosition(file, trackid, endMarker, endFrame);
	return AF_SUCCEED;
}

/*
	Get loop end frame.
*/

AFframecount afGetLoopEndFrame (AFfilehandle file, int instid, int loopid)
{
	Loop *loop = getLoop(file, instid, loopid, false);

	if (!loop)
		return -1;

	int trackid = loop->trackid;
	int endMarker = loop->endMarker;

	return afGetMarkPosition(file, trackid, endMarker);
}
