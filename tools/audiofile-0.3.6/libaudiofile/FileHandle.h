/*
	Audio File Library
	Copyright (C) 2010-2011, Michael Pruett <michael@68k.org>

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

#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include "afinternal.h"
#include <stdint.h>

class File;
class Tag;
struct Instrument;
struct Miscellaneous;
struct Track;

struct _AFfilehandle
{
	static _AFfilehandle *create(int fileFormat);

	int m_valid;	// _AF_VALID_FILEHANDLE
	int m_access;	// _AF_READ_ACCESS or _AF_WRITE_ACCESS

	bool m_seekok;

	File *m_fh;

	char *m_fileName;

	int m_fileFormat;

	int m_trackCount;
	Track *m_tracks;

	int m_instrumentCount;
	Instrument *m_instruments;

	int m_miscellaneousCount;
	Miscellaneous *m_miscellaneous;

private:
	int m_formatByteOrder;

	status copyTracksFromSetup(AFfilesetup setup);
	status copyInstrumentsFromSetup(AFfilesetup setup);
	status copyMiscellaneousFromSetup(AFfilesetup setup);

public:
	virtual ~_AFfilehandle();

	virtual int getVersion() { return 0; }
	virtual status readInit(AFfilesetup) = 0;
	virtual status writeInit(AFfilesetup) = 0;
	virtual status update() = 0;
	virtual bool isInstrumentParameterValid(AUpvlist, int) { return false; }

	bool checkCanRead();
	bool checkCanWrite();

	Track *allocateTrack();
	Track *getTrack(int trackID = AF_DEFAULT_TRACK);
	Instrument *getInstrument(int instrumentID);
	Miscellaneous *getMiscellaneous(int miscellaneousID);

protected:
	_AFfilehandle();

	status initFromSetup(AFfilesetup setup);

	void setFormatByteOrder(int byteOrder) { m_formatByteOrder = byteOrder; }

	bool readU8(uint8_t *);
	bool readS8(int8_t *);
	bool readU16(uint16_t *);
	bool readS16(int16_t *);
	bool readU32(uint32_t *);
	bool readS32(int32_t *);
	bool readU64(uint64_t *);
	bool readS64(int64_t *);
	bool readFloat(float *);
	bool readDouble(double *);

	bool writeU8(const uint8_t *);
	bool writeS8(const int8_t *);
	bool writeU16(const uint16_t *);
	bool writeS16(const int16_t *);
	bool writeU32(const uint32_t *);
	bool writeS32(const int32_t *);
	bool writeU64(const uint64_t *);
	bool writeS64(const int64_t *);
	bool writeFloat(const float *);
	bool writeDouble(const double *);

	bool readTag(Tag *t);
	bool writeTag(const Tag *t);
};

#endif
