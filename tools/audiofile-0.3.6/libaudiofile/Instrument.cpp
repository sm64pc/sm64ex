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
	Instrument.cpp

	Info about instrument parameters:

	Each unit has an array of _InstParamInfo structures, one for
	each instrument parameter.  Each of these structures describes
	the inst parameters.

	id: a 4-byte id as in AIFF file
	type: data type AU_PVLIST_*
	name: text name
	defaultValue: default value, to which it is set when a file with
		instruments is first opened for writing.

	Each inst has only an array of values (_AFPVu's).  Each value in the
	instrument's array is the value of the corresponding index into the
	unit's instparaminfo array.

	So for a unit u and an instrument i, u.instparam[N] describes
	the parameter whose value is given in i.value[N].
*/

#include "config.h"

#include "FileHandle.h"
#include "Instrument.h"
#include "Setup.h"
#include "afinternal.h"
#include "audiofile.h"
#include "units.h"
#include "util.h"

#include <stdlib.h>

bool InstrumentSetup::allocateLoops(int count)
{
	freeLoops();
	loops = (LoopSetup *) _af_calloc(count, sizeof (LoopSetup));
	if (loops)
	{
		loopCount = count;
		return true;
	}
	return false;
}

void InstrumentSetup::freeLoops()
{
	if (loops)
		free(loops);
	loops = NULL;
	loopCount = 0;
}

/*
	Initialize instrument id list for audio file.
*/
void afInitInstIDs (AFfilesetup setup, const int *instids, int ninsts)
{
	if (!_af_filesetup_ok(setup))
		return;

	if (!_af_unique_ids(instids, ninsts, "instrument", AF_BAD_INSTID))
		return;

	_af_setup_free_instruments(setup);

	setup->instrumentCount = ninsts;
	setup->instrumentSet = true;

	setup->instruments = _af_instsetup_new(setup->instrumentCount);

	for (int i=0; i < setup->instrumentCount; i++)
		setup->instruments[i].id = instids[i];
}

int afGetInstIDs (AFfilehandle file, int *instids)
{
	if (!_af_filehandle_ok(file))
		return -1;

	if (instids)
		for (int i=0; i < file->m_instrumentCount; i++)
			instids[i] = file->m_instruments[i].id;

	return file->m_instrumentCount;
}

/*
	This routine checks and sets instrument parameters.
	npv is number of valid AUpvlist pairs.
*/
void _af_instparam_set (AFfilehandle file, int instid, AUpvlist pvlist, int npv)
{
	if (!_af_filehandle_ok(file))
		return;

	if (!file->checkCanWrite())
		return;

	Instrument *instrument = file->getInstrument(instid);
	if (!instrument)
		return;

	if (AUpvgetmaxitems(pvlist) < npv)
	npv = AUpvgetmaxitems(pvlist);

	for (int i=0; i < npv; i++)
	{
		int	param;

		AUpvgetparam(pvlist, i, &param);

		int j;
		if ((j = _af_instparam_index_from_id(file->m_fileFormat, param)) == -1)
			/* no parameter with that id; ignore */
			continue;

		if (!file->isInstrumentParameterValid(pvlist, i))
			/* bad parameter value; ignore */
			continue;

		int	type = _af_units[file->m_fileFormat].instrumentParameters[j].type;

		switch (type)
		{
			case AU_PVTYPE_LONG:
				AUpvgetval(pvlist, i, &instrument->values[j].l);
				break;
			case AU_PVTYPE_DOUBLE:
				AUpvgetval(pvlist, i, &instrument->values[j].d);
				break;
			case AU_PVTYPE_PTR:
				AUpvgetval(pvlist, i, &instrument->values[j].v);
				break;
			default:
				return;
		}
	}
}

void afSetInstParams (AFfilehandle file, int instid, AUpvlist pvlist, int npv)
{
	_af_instparam_set(file, instid, pvlist, npv);
}

void afSetInstParamLong (AFfilehandle file, int instid, int param, long value)
{
	AUpvlist pvlist = AUpvnew(1);

	AUpvsetparam(pvlist, 0, param);
	AUpvsetvaltype(pvlist, 0, AU_PVTYPE_LONG);
	AUpvsetval(pvlist, 0, &value);

	_af_instparam_set(file, instid, pvlist, 1);

	AUpvfree(pvlist);
}

/*
	This routine gets instrument parameters.
	npv is number of valid AUpvlist pairs
*/
void _af_instparam_get (AFfilehandle file, int instid, AUpvlist pvlist, int npv,
	bool forceLong)
{
	if (!_af_filehandle_ok(file))
		return;

	Instrument *instrument = file->getInstrument(instid);
	if (!instrument)
		return;

	if (AUpvgetmaxitems(pvlist) < npv)
		npv = AUpvgetmaxitems(pvlist);

	for (int i=0; i < npv; i++)
	{
		int param;
		AUpvgetparam(pvlist, i, &param);

		int j;
		if ((j = _af_instparam_index_from_id(file->m_fileFormat, param)) == -1)
			/* no parameter with that id; ignore */
			continue;

		int type = _af_units[file->m_fileFormat].instrumentParameters[j].type;

		/*
			forceLong is true when this routine called by
			afGetInstParamLong().
		*/
		if (forceLong && type != AU_PVTYPE_LONG)
		{
			_af_error(AF_BAD_INSTPTYPE, "type of instrument parameter %d is not AU_PVTYPE_LONG", param);
			continue;
		}

		AUpvsetvaltype(pvlist, i, type);

		switch (type)
		{
			case AU_PVTYPE_LONG:
				AUpvsetval(pvlist, i, &instrument->values[j].l);
				break;
			case AU_PVTYPE_DOUBLE:
				AUpvsetval(pvlist, i, &instrument->values[j].d);
				break;
			case AU_PVTYPE_PTR:
				AUpvsetval(pvlist, i, &instrument->values[j].v);
				break;

			default:
				_af_error(AF_BAD_INSTPTYPE, "invalid instrument parameter type %d", type);
				return;
		}
	}
}

/*
	afGetInstParams -- get a parameter-value array containing
	instrument parameters for the specified instrument chunk
*/
void afGetInstParams (AFfilehandle file, int inst, AUpvlist pvlist, int npv)
{
	_af_instparam_get(file, inst, pvlist, npv, false);
}

long afGetInstParamLong (AFfilehandle file, int inst, int param)
{
	long val;
	AUpvlist pvlist = AUpvnew(1);

	AUpvsetparam(pvlist, 0, param);
	AUpvsetvaltype(pvlist, 0, AU_PVTYPE_LONG);

	_af_instparam_get(file, inst, pvlist, 1, true);

	AUpvgetval(pvlist, 0, &val);
	AUpvfree(pvlist);

	return(val);
}

/*
	Search _af_units[fileFormat].instrumentParameters for the instrument
	parameter with the specified id.

	Report an error and return -1 if no such instrument parameter
	exists.
*/

int _af_instparam_index_from_id (int filefmt, int id)
{
	int i;

	for (i = 0; i < _af_units[filefmt].instrumentParameterCount; i++)
		if (_af_units[filefmt].instrumentParameters[i].id == id)
			break;

	if (i == _af_units[filefmt].instrumentParameterCount)
	{
		_af_error(AF_BAD_INSTPID, "invalid instrument parameter id %d",
			id);
		return -1;
	}

	return i;
}

Loop *Instrument::getLoop(int loopID)
{
	for (int i=0; i<loopCount; i++)
		if (loops[i].id == loopID)
			return &loops[i];
	
	_af_error(AF_BAD_LOOPID, "no loop with id %d for instrument %d\n",
		loopID, id);
	return NULL;
}
