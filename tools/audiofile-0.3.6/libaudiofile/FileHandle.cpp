/*
	Audio File Library
	Copyright (C) 2010-2012, Michael Pruett <michael@68k.org>
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

#include "config.h"
#include "FileHandle.h"

#include "afinternal.h"
#include "audiofile.h"
#include "byteorder.h"
#include <stdlib.h>
#include <assert.h>

#include "AIFF.h"
#include "AVR.h"
#include "CAF.h"
#include "FLACFile.h"
#include "IFF.h"
#include "IRCAM.h"
#include "NeXT.h"
#include "NIST.h"
#include "Raw.h"
#include "SampleVision.h"
#include "VOC.h"
#include "WAVE.h"

#include "File.h"
#include "Instrument.h"
#include "Setup.h"
#include "Tag.h"
#include "Track.h"
#include "units.h"
#include "util.h"

static void freeInstParams (AFPVu *values, int fileFormat)
{
	if (!values)
		return;

	int parameterCount = _af_units[fileFormat].instrumentParameterCount;

	for (int i=0; i<parameterCount; i++)
	{
		if (_af_units[fileFormat].instrumentParameters[i].type == AU_PVTYPE_PTR)
			free(values[i].v);
	}

	free(values);
}

_AFfilehandle *_AFfilehandle::create(int fileFormat)
{
	switch (fileFormat)
	{
		case AF_FILE_RAWDATA:
			return new RawFile();
		case AF_FILE_AIFF:
		case AF_FILE_AIFFC:
			return new AIFFFile();
		case AF_FILE_NEXTSND:
			return new NeXTFile();
		case AF_FILE_WAVE:
			return new WAVEFile();
		case AF_FILE_BICSF:
			return new IRCAMFile();
		case AF_FILE_AVR:
			return new AVRFile();
		case AF_FILE_IFF_8SVX:
			return new IFFFile();
		case AF_FILE_SAMPLEVISION:
			return new SampleVisionFile();
		case AF_FILE_VOC:
			return new VOCFile();
		case AF_FILE_NIST_SPHERE:
			return new NISTFile();
		case AF_FILE_CAF:
			return new CAFFile();
		case AF_FILE_FLAC:
			return new FLACFile();
		default:
			return NULL;
	}
}

_AFfilehandle::_AFfilehandle()
{
	m_valid = _AF_VALID_FILEHANDLE;
	m_access = 0;
	m_seekok = false;
	m_fh = NULL;
	m_fileName = NULL;
	m_fileFormat = AF_FILE_UNKNOWN;
	m_trackCount = 0;
	m_tracks = NULL;
	m_instrumentCount = 0;
	m_instruments = NULL;
	m_miscellaneousCount = 0;
	m_miscellaneous = NULL;
	m_formatByteOrder = 0;
}

_AFfilehandle::~_AFfilehandle()
{
	m_valid = 0;

	free(m_fileName);

	delete [] m_tracks;
	m_tracks = NULL;
	m_trackCount = 0;

	if (m_instruments)
	{
		for (int i=0; i<m_instrumentCount; i++)
		{
			free(m_instruments[i].loops);
			m_instruments[i].loops = NULL;
			m_instruments[i].loopCount = 0;

			freeInstParams(m_instruments[i].values, m_fileFormat);
			m_instruments[i].values = NULL;
		}

		free(m_instruments);
		m_instruments = NULL;
	}
	m_instrumentCount = 0;

	if (m_miscellaneous)
	{
		for (int i=0; i<m_miscellaneousCount; i++)
			free(m_miscellaneous[i].buffer);
		free(m_miscellaneous);
		m_miscellaneous = NULL;
	}
	m_miscellaneousCount = 0;
}

Track *_AFfilehandle::allocateTrack()
{
	assert(!m_trackCount);
	assert(!m_tracks);

	m_trackCount = 1;
	m_tracks = new Track[1];
	return m_tracks;
}

Track *_AFfilehandle::getTrack(int trackID)
{
	for (int i=0; i<m_trackCount; i++)
		if (m_tracks[i].id == trackID)
			return &m_tracks[i];

	_af_error(AF_BAD_TRACKID, "bad track id %d", trackID);

	return NULL;
}

bool _AFfilehandle::checkCanRead()
{
	if (m_access != _AF_READ_ACCESS)
	{
		_af_error(AF_BAD_NOREADACC, "file not opened for read access");
		return false;
	}

	return true;
}

bool _AFfilehandle::checkCanWrite()
{
	if (m_access != _AF_WRITE_ACCESS)
	{
		_af_error(AF_BAD_NOWRITEACC, "file not opened for write access");
		return false;
	}

	return true;
}

Instrument *_AFfilehandle::getInstrument(int instrumentID)
{
	for (int i = 0; i < m_instrumentCount; i++)
		if (m_instruments[i].id == instrumentID)
			return &m_instruments[i];

	_af_error(AF_BAD_INSTID, "invalid instrument id %d", instrumentID);
	return NULL;
}

Miscellaneous *_AFfilehandle::getMiscellaneous(int miscellaneousID)
{
	for (int i=0; i<m_miscellaneousCount; i++)
	{
		if (m_miscellaneous[i].id == miscellaneousID)
			return &m_miscellaneous[i];
	}

	_af_error(AF_BAD_MISCID, "bad miscellaneous id %d", miscellaneousID);

	return NULL;
}

status _AFfilehandle::initFromSetup(AFfilesetup setup)
{
	if (copyTracksFromSetup(setup) == AF_FAIL)
		return AF_FAIL;
	if (copyInstrumentsFromSetup(setup) == AF_FAIL)
		return AF_FAIL;
	if (copyMiscellaneousFromSetup(setup) == AF_FAIL)
		return AF_FAIL;
	return AF_SUCCEED;
}

status _AFfilehandle::copyTracksFromSetup(AFfilesetup setup)
{
	if ((m_trackCount = setup->trackCount) == 0)
	{
		m_tracks = NULL;
		return AF_SUCCEED;
	}

	m_tracks = new Track[m_trackCount];
	if (!m_tracks)
		return AF_FAIL;

	for (int i=0; i<m_trackCount; i++)
	{
		Track *track = &m_tracks[i];
		TrackSetup *trackSetup = &setup->tracks[i];

		track->id = trackSetup->id;
		track->f = trackSetup->f;

		if (track->copyMarkers(trackSetup) == AF_FAIL)
			return AF_FAIL;

		track->hasAESData = trackSetup->aesDataSet;
	}

	return AF_SUCCEED;
}

status _AFfilehandle::copyInstrumentsFromSetup(AFfilesetup setup)
{
	if ((m_instrumentCount = setup->instrumentCount) == 0)
	{
		m_instruments = NULL;
		return AF_SUCCEED;
	}

	m_instruments = static_cast<Instrument *>(_af_calloc(m_instrumentCount,
		sizeof (Instrument)));
	if (!m_instruments)
		return AF_FAIL;

	for (int i=0; i<m_instrumentCount; i++)
	{
		m_instruments[i].id = setup->instruments[i].id;

		// Copy loops.
		if ((m_instruments[i].loopCount = setup->instruments[i].loopCount) == 0)
		{
			m_instruments[i].loops = NULL;
		}
		else
		{
			m_instruments[i].loops =
				static_cast<Loop *>(_af_calloc(m_instruments[i].loopCount,
					sizeof (Loop)));
			if (!m_instruments[i].loops)
				return AF_FAIL;
			for (int j=0; j<m_instruments[i].loopCount; j++)
			{
				Loop *loop = &m_instruments[i].loops[j];
				loop->id = setup->instruments[i].loops[j].id;
				loop->mode = AF_LOOP_MODE_NOLOOP;
				loop->count = 0;
				loop->trackid = AF_DEFAULT_TRACK;
				loop->beginMarker = 2*j + 1;
				loop->endMarker = 2*j + 2;
			}
		}

		int instParamCount;
		// Copy instrument parameters.
		if ((instParamCount = _af_units[setup->fileFormat].instrumentParameterCount) == 0)
		{
			m_instruments[i].values = NULL;
		}
		else
		{
			m_instruments[i].values =
				static_cast<AFPVu *>(_af_calloc(instParamCount, sizeof (AFPVu)));
			if (!m_instruments[i].values)
				return AF_FAIL;
			for (int j=0; j<instParamCount; j++)
			{
				m_instruments[i].values[j] = _af_units[setup->fileFormat].instrumentParameters[j].defaultValue;
			}
		}
	}

	return AF_SUCCEED;
}

status _AFfilehandle::copyMiscellaneousFromSetup(AFfilesetup setup)
{
	if ((m_miscellaneousCount = setup->miscellaneousCount) == 0)
	{
		m_miscellaneous = NULL;
		return AF_SUCCEED;
	}

	m_miscellaneous = static_cast<Miscellaneous *>(_af_calloc(m_miscellaneousCount,
		sizeof (Miscellaneous)));
	if (!m_miscellaneous)
		return AF_FAIL;

	for (int i=0; i<m_miscellaneousCount; i++)
	{
		m_miscellaneous[i].id = setup->miscellaneous[i].id;
		m_miscellaneous[i].type = setup->miscellaneous[i].type;
		m_miscellaneous[i].size = setup->miscellaneous[i].size;
		m_miscellaneous[i].position = 0;
		m_miscellaneous[i].buffer = NULL;
	}

	return AF_SUCCEED;
}

template <typename T>
static bool readValue(File *f, T *value)
{
	return f->read(value, sizeof (*value)) == sizeof (*value);
}

template <typename T>
static bool writeValue(File *f, const T *value)
{
	return f->write(value, sizeof (*value)) == sizeof (*value);
}

template <typename T>
static T swapValue(T value, int order)
{
	if (order == AF_BYTEORDER_BIGENDIAN)
		return bigToHost(value);
	else if (order == AF_BYTEORDER_LITTLEENDIAN)
		return littleToHost(value);
	return value;
}

template <typename T>
static bool readSwap(File *f, T *value, int order)
{
	if (!readValue(f, value)) return false;
	*value = swapValue(*value, order);
	return true;
}

template <typename T>
static bool writeSwap(File *f, const T *value, int order)
{
	T t = swapValue(*value, order);
	return writeValue(f, &t);
}

bool _AFfilehandle::readU8(uint8_t *v) { return readValue(m_fh, v); }
bool _AFfilehandle::readS8(int8_t *v) { return readValue(m_fh, v); }

bool _AFfilehandle::readU16(uint16_t *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readS16(int16_t *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readU32(uint32_t *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readS32(int32_t *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readU64(uint64_t *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readS64(int64_t *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readFloat(float *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readDouble(double *v)
{
	return readSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeU8(const uint8_t *v) { return writeValue(m_fh, v); }
bool _AFfilehandle::writeS8(const int8_t *v) { return writeValue(m_fh, v); }

bool _AFfilehandle::writeU16(const uint16_t *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeS16(const int16_t *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeU32(const uint32_t *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeS32(const int32_t *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeU64(const uint64_t *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeS64(const int64_t *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeFloat(const float *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::writeDouble(const double *v)
{
	return writeSwap(m_fh, v, m_formatByteOrder);
}

bool _AFfilehandle::readTag(Tag *t)
{
	uint32_t v;
	if (m_fh->read(&v, sizeof (v)) == sizeof (v))
	{
		*t = Tag(v);
		return true;
	}
	return false;
}

bool _AFfilehandle::writeTag(const Tag *t)
{
	uint32_t v = t->value();
	return m_fh->write(&v, sizeof (v)) == sizeof (v);
}
