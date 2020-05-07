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
	Instrument.h

	This file declares routines for dealing with instruments.
*/

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "afinternal.h"
#include "audiofile.h"
#include "aupvlist.h"

struct LoopSetup;
struct Loop;

struct InstrumentSetup
{
	int	id;

	int loopCount;
	LoopSetup *loops;

	bool loopSet;

	bool allocateLoops(int count);
	void freeLoops();
};

struct Instrument
{
	int id;

	int loopCount;
	Loop *loops;

	AFPVu *values;

	Loop *getLoop(int loopID);
};

void _af_instparam_get (AFfilehandle file, int instid, AUpvlist pvlist,
	int npv, bool forceLong);

void _af_instparam_set (AFfilehandle file, int instid, AUpvlist pvlist,
	int npv);

int _af_instparam_index_from_id (int fileFormat, int id);

#endif /* INSTRUMENT_H */
