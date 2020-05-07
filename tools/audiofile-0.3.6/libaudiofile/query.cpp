/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>

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
	query.cpp

	This file contains the implementation of the Audio File Library's
	query mechanism.  Querying through the afQuery calls can allow the
	programmer to determine the capabilities and characteristics of
	the Audio File Library implementation and its supported formats.
*/

#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "audiofile.h"
#include "afinternal.h"
#include "aupvlist.h"
#include "error.h"
#include "util.h"
#include "units.h"
#include "compression.h"
#include "Instrument.h"

AUpvlist _afQueryFileFormat (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryInstrument (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryInstrumentParameter (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryLoop (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryMarker (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryMiscellaneous (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryCompression (int arg1, int arg2, int arg3, int arg4);
AUpvlist _afQueryCompressionParameter (int arg1, int arg2, int arg3, int arg4);

AUpvlist afQuery (int querytype, int arg1, int arg2, int arg3, int arg4)
{
	switch (querytype)
	{
		case AF_QUERYTYPE_INST:
			return _afQueryInstrument(arg1, arg2, arg3, arg4);
		case AF_QUERYTYPE_INSTPARAM:
			return _afQueryInstrumentParameter(arg1, arg2, arg3, arg4);
		case AF_QUERYTYPE_LOOP:
			return _afQueryLoop(arg1, arg2, arg3, arg4);
		case AF_QUERYTYPE_FILEFMT:
			return _afQueryFileFormat(arg1, arg2, arg3, arg4);
		case AF_QUERYTYPE_COMPRESSION:
			return _afQueryCompression(arg1, arg2, arg3, arg4);
		case AF_QUERYTYPE_COMPRESSIONPARAM:
			/* FIXME: This selector is not implemented. */
			return AU_NULL_PVLIST;
		case AF_QUERYTYPE_MISC:
			/* FIXME: This selector is not implemented. */
			return AU_NULL_PVLIST;
		case AF_QUERYTYPE_MARK:
			return _afQueryMarker(arg1, arg2, arg3, arg4);
	}

	_af_error(AF_BAD_QUERYTYPE, "bad query type");
	return AU_NULL_PVLIST;
}

/* ARGSUSED3 */
AUpvlist _afQueryFileFormat (int arg1, int arg2, int arg3, int arg4)
{
	switch (arg1)
	{
		/* The following select only on arg1. */
		case AF_QUERY_ID_COUNT:
		{
			int	count = 0, idx;
			for (idx = 0; idx < _AF_NUM_UNITS; idx++)
				if (_af_units[idx].implemented)
					count++;
			return _af_pv_long(count);
		}
		/* NOTREACHED */
		break;

		case AF_QUERY_IDS:
		{
			int	count = 0, idx;
			int	*buffer;

			buffer = (int *) _af_calloc(_AF_NUM_UNITS, sizeof (int));
			if (buffer == NULL)
				return AU_NULL_PVLIST;

			for (idx = 0; idx < _AF_NUM_UNITS; idx++)
				if (_af_units[idx].implemented)
					buffer[count++] = idx;

			if (count == 0)
			{
				free(buffer);
				return AU_NULL_PVLIST;
			}

			return _af_pv_pointer(buffer);
		}
		/* NOTREACHED */
		break;

		/* The following select on arg2. */
		case AF_QUERY_LABEL:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(_af_units[arg2].label));

		case AF_QUERY_NAME:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(_af_units[arg2].name));

		case AF_QUERY_DESC:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(_af_units[arg2].description));

		case AF_QUERY_IMPLEMENTED:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return _af_pv_long(0);
			return _af_pv_long(_af_units[arg2].implemented);

		/* The following select on arg3. */
		case AF_QUERY_SAMPLE_FORMATS:
			if (arg3 < 0 || arg3 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			switch (arg2)
			{
				case AF_QUERY_DEFAULT:
					return _af_pv_long(_af_units[arg3].defaultSampleFormat);
				default:
					break;
			}
			/* NOTREACHED */
			break;

		case AF_QUERY_SAMPLE_SIZES:
			if (arg3 < 0 || arg3 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;

			switch (arg2)
			{
				case AF_QUERY_DEFAULT:
					return _af_pv_long(_af_units[arg3].defaultSampleWidth);
				default:
					break;
			}
			/* NOTREACHED */
			break;

		case AF_QUERY_COMPRESSION_TYPES:
		{
			int	idx, count;
			int	*buffer;

			if (arg3 < 0 || arg3 >= _AF_NUM_UNITS)
			{
				_af_error(AF_BAD_QUERY,
					"unrecognized file format %d", arg3);
				return AU_NULL_PVLIST;
			}

			switch (arg2)
			{
				case AF_QUERY_VALUE_COUNT:
					count = _af_units[arg3].compressionTypeCount;
					return _af_pv_long(count);

				case AF_QUERY_VALUES:
					count = _af_units[arg3].compressionTypeCount;
					if (count == 0)
						return AU_NULL_PVLIST;

					buffer = (int *) _af_calloc(count, sizeof (int));
					if (buffer == NULL)
						return AU_NULL_PVLIST;

					for (idx = 0; idx < count; idx++)
					{
						buffer[idx] = _af_units[arg3].compressionTypes[idx];
					}

					return _af_pv_pointer(buffer);
			}
		}
		break;
	}

	_af_error(AF_BAD_QUERY, "bad query selector");
	return AU_NULL_PVLIST;
}

long afQueryLong (int querytype, int arg1, int arg2, int arg3, int arg4)
{
	AUpvlist	list;
	int		type;
	long		value;

	list = afQuery(querytype, arg1, arg2, arg3, arg4);
	if (list == AU_NULL_PVLIST)
		return -1;
	AUpvgetvaltype(list, 0, &type);
	if (type != AU_PVTYPE_LONG)
		return -1;
	AUpvgetval(list, 0, &value);
	AUpvfree(list);
	return value;
}

double afQueryDouble (int querytype, int arg1, int arg2, int arg3, int arg4)
{
	AUpvlist	list;
	int		type;
	double		value;

	list = afQuery(querytype, arg1, arg2, arg3, arg4);
	if (list == AU_NULL_PVLIST)
		return -1;
	AUpvgetvaltype(list, 0, &type);
	if (type != AU_PVTYPE_DOUBLE)
		return -1;
	AUpvgetval(list, 0, &value);
	AUpvfree(list);
	return value;
}

void *afQueryPointer (int querytype, int arg1, int arg2, int arg3, int arg4)
{
	AUpvlist	list;
	int		type;
	void		*value;

	list = afQuery(querytype, arg1, arg2, arg3, arg4);
	if (list == AU_NULL_PVLIST)
		return NULL;
	AUpvgetvaltype(list, 0, &type);
	if (type != AU_PVTYPE_PTR)
		return NULL;
	AUpvgetval(list, 0, &value);
	AUpvfree(list);
	return value;
}

/* ARGSUSED3 */
AUpvlist _afQueryInstrumentParameter (int arg1, int arg2, int arg3, int arg4)
{
	switch (arg1)
	{
		/* For the following query types, arg2 is the file format. */
		case AF_QUERY_SUPPORTED:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_long(_af_units[arg2].instrumentParameterCount != 0);

		case AF_QUERY_ID_COUNT:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_long(_af_units[arg2].instrumentParameterCount);

		case AF_QUERY_IDS:
		{
			int	count;
			int	*buffer;

			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			count = _af_units[arg2].instrumentParameterCount;
			if (count == 0)
				return AU_NULL_PVLIST;
			buffer = (int *) _af_calloc(count, sizeof (int));
			if (buffer == NULL)
				return AU_NULL_PVLIST;
			for (int i=0; i<count; i++)
				buffer[i] = _af_units[arg2].instrumentParameters[i].id;
			return _af_pv_pointer(buffer);
		}
		/* NOTREACHED */
		break;

		/*
			For the next few query types, arg2 is the file
			format and arg3 is the instrument parameter id.
		*/
		case AF_QUERY_TYPE:
		{
			int	idx;

			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;

			idx = _af_instparam_index_from_id(arg2, arg3);
			if (idx<0)
				return AU_NULL_PVLIST;
			return _af_pv_long(_af_units[arg2].instrumentParameters[idx].type);
		}

		case AF_QUERY_NAME:
		{
			int	idx;

			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			idx = _af_instparam_index_from_id(arg2, arg3);
			if (idx < 0)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(_af_units[arg2].instrumentParameters[idx].name));
		}

		case AF_QUERY_DEFAULT:
		{
			int	idx;

			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			idx = _af_instparam_index_from_id(arg2, arg3);
			if (idx >= 0)
			{
				AUpvlist	ret = AUpvnew(1);
				AUpvsetparam(ret, 0, _af_units[arg2].instrumentParameters[idx].id);
				AUpvsetvaltype(ret, 0, _af_units[arg2].instrumentParameters[idx].type);
				AUpvsetval(ret, 0, const_cast<AFPVu *>(&_af_units[arg2].instrumentParameters[idx].defaultValue));
				return ret;
			}
			return AU_NULL_PVLIST;
		}
	}

	_af_error(AF_BAD_QUERY, "bad query selector");
	return AU_NULL_PVLIST;
}

/* ARGSUSED2 */
AUpvlist _afQueryLoop (int arg1, int arg2, int arg3, int arg4)
{
	if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
		return AU_NULL_PVLIST;

	switch (arg1)
	{
		case AF_QUERY_SUPPORTED:
			return _af_pv_long(_af_units[arg2].loopPerInstrumentCount != 0);
		case AF_QUERY_MAX_NUMBER:
			return _af_pv_long(_af_units[arg2].loopPerInstrumentCount);
	}

	_af_error(AF_BAD_QUERY, "bad query selector");
	return AU_NULL_PVLIST;
}

/* ARGSUSED2 */
AUpvlist _afQueryInstrument (int arg1, int arg2, int arg3, int arg4)
{
	switch (arg1)
	{
		case AF_QUERY_SUPPORTED:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_long(_af_units[arg2].instrumentCount != 0);

		case AF_QUERY_MAX_NUMBER:
			if (arg2 < 0 || arg2 >= _AF_NUM_UNITS)
				return AU_NULL_PVLIST;
			return _af_pv_long(_af_units[arg2].instrumentCount);
	}

	_af_error(AF_BAD_QUERY, "bad query selector");
	return AU_NULL_PVLIST;
}

/* ARGSUSED0 */
AUpvlist _afQueryMiscellaneous (int arg1, int arg2, int arg3, int arg4)
{
	_af_error(AF_BAD_NOT_IMPLEMENTED, "not implemented yet");
	return AU_NULL_PVLIST;
}

/* ARGSUSED2 */
AUpvlist _afQueryMarker (int arg1, int arg2, int arg3, int arg4)
{
	switch (arg1)
	{
		case AF_QUERY_SUPPORTED:
			return _af_pv_long(_af_units[arg2].markerCount != 0);
		case AF_QUERY_MAX_NUMBER:
			return _af_pv_long(_af_units[arg2].markerCount);
	}

	_af_error(AF_BAD_QUERY, "bad query selector");
	return AU_NULL_PVLIST;
}

/* ARGSUSED0 */
AUpvlist _afQueryCompression (int arg1, int arg2, int arg3, int arg4)
{
	const CompressionUnit *unit = NULL;

	switch (arg1)
	{
		case AF_QUERY_ID_COUNT:
		{
			int count = 0;
			for (int i = 0; i < _AF_NUM_COMPRESSION; i++)
				if (_af_compression[i].implemented)
					count++;
			return _af_pv_long(count);
		}

		case AF_QUERY_IDS:
		{
			int *buf = (int *) _af_calloc(_AF_NUM_COMPRESSION, sizeof (int));
			if (!buf)
				return AU_NULL_PVLIST;

			int count = 0;
			for (int i = 0; i < _AF_NUM_COMPRESSION; i++)
			{
				if (_af_compression[i].implemented)
					buf[count++] = _af_compression[i].compressionID;
			}
			return _af_pv_pointer(buf);
		}

		case AF_QUERY_IMPLEMENTED:
			unit = _af_compression_unit_from_id(arg2);
			if (!unit)
				return _af_pv_long(0);
			return _af_pv_long(unit->implemented);

		case AF_QUERY_NATIVE_SAMPFMT:
			unit = _af_compression_unit_from_id(arg2);
			if (!unit)
				return AU_NULL_PVLIST;
			return _af_pv_long(unit->nativeSampleFormat);

		case AF_QUERY_NATIVE_SAMPWIDTH:
			unit = _af_compression_unit_from_id(arg2);
			if (!unit)
				return AU_NULL_PVLIST;
			return _af_pv_long(unit->nativeSampleWidth);

		case AF_QUERY_LABEL:
			unit = _af_compression_unit_from_id(arg2);
			if (!unit)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(unit->label));

		case AF_QUERY_NAME:
			unit = _af_compression_unit_from_id(arg2);
			if (!unit)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(unit->shortname));

		case AF_QUERY_DESC:
			unit = _af_compression_unit_from_id(arg2);
			if (!unit)
				return AU_NULL_PVLIST;
			return _af_pv_pointer(const_cast<char *>(unit->name));
	}

	_af_error(AF_BAD_QUERY, "unrecognized query selector %d\n", arg1);
	return AU_NULL_PVLIST;
}
