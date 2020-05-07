/*
	Audio File Library
	Copyright (C) 1998-2000, 2011-2013, Michael Pruett <michael@68k.org>

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
	NeXT.cpp

	This file contains routines for reading and writing NeXT/Sun
	.snd/.au sound files.
*/

#include "config.h"
#include "NeXT.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "File.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "byteorder.h"
#include "util.h"

const int _af_next_compression_types[_AF_NEXT_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW
};

static const _AFfilesetup nextDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_NEXTSND,	/* fileFormat */
	true,			/* trackSet */
	true,			/* instrumentSet */
	true,			/* miscellaneousSet */
	1,			/* trackCount */
	NULL,			/* tracks */
	0,			/* instrumentCount */
	NULL,			/* instruments */
	0,			/* miscellaneousCount */
	NULL			/* miscellaneous */
};

enum
{
	_AU_FORMAT_UNSPECIFIED = 0,
	_AU_FORMAT_MULAW_8 = 1,		/* CCITT G.711 mu-law 8-bit */
	_AU_FORMAT_LINEAR_8 = 2,
	_AU_FORMAT_LINEAR_16 = 3,
	_AU_FORMAT_LINEAR_24 = 4,
	_AU_FORMAT_LINEAR_32 = 5,
	_AU_FORMAT_FLOAT = 6,
	_AU_FORMAT_DOUBLE = 7,
	_AU_FORMAT_INDIRECT = 8,
	_AU_FORMAT_NESTED = 9,
	_AU_FORMAT_DSP_CORE = 10,
	_AU_FORMAT_DSP_DATA_8 = 11,	/* 8-bit fixed point */
	_AU_FORMAT_DSP_DATA_16 = 12,	/* 16-bit fixed point */
	_AU_FORMAT_DSP_DATA_24 = 13,	/* 24-bit fixed point */
	_AU_FORMAT_DSP_DATA_32 = 14,	/* 32-bit fixed point */
	_AU_FORMAT_DISPLAY = 16,
	_AU_FORMAT_MULAW_SQUELCH = 17,	/* 8-bit mu-law, squelched */
	_AU_FORMAT_EMPHASIZED = 18,
	_AU_FORMAT_COMPRESSED = 19,
	_AU_FORMAT_COMPRESSED_EMPHASIZED = 20,
	_AU_FORMAT_DSP_COMMANDS = 21,
	_AU_FORMAT_DSP_COMMANDS_SAMPLES = 22,
	_AU_FORMAT_ADPCM_G721 = 23,	/* CCITT G.721 ADPCM 32 kbits/s */
	_AU_FORMAT_ADPCM_G722 = 24,	/* CCITT G.722 ADPCM */
	_AU_FORMAT_ADPCM_G723_3 = 25,	/* CCITT G.723 ADPCM 24 kbits/s */
	_AU_FORMAT_ADPCM_G723_5 = 26,	/* CCITT G.723 ADPCM 40 kbits/s */
	_AU_FORMAT_ALAW_8 = 27,		/* CCITT G.711 a-law */
	_AU_FORMAT_AES = 28,
	_AU_FORMAT_DELTA_MULAW_8 = 29
};

static const uint32_t _AU_LENGTH_UNSPECIFIED = 0xffffffff;

NeXTFile::NeXTFile()
{
	setFormatByteOrder(AF_BYTEORDER_BIGENDIAN);
}

status NeXTFile::readInit(AFfilesetup setup)
{
	uint32_t id, offset, length, encoding, sampleRate, channelCount;

	m_fh->seek(0, File::SeekFromBeginning);

	m_fh->read(&id, 4);
	assert(!memcmp(&id, ".snd", 4));

	readU32(&offset);
	readU32(&length);
	readU32(&encoding);
	readU32(&sampleRate);
	readU32(&channelCount);

	if (!channelCount)
	{
		_af_error(AF_BAD_CHANNELS, "invalid file with 0 channels");
		return AF_FAIL;
	}

	Track *track = allocateTrack();
	if (!track)
		return AF_FAIL;

	track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;
	track->f.sampleRate = sampleRate;
	track->f.channelCount = channelCount;
	track->f.framesPerPacket = 1;

	/* Override the compression type later if necessary. */
	track->f.compressionType = AF_COMPRESSION_NONE;

	track->fpos_first_frame = offset;

	off_t lengthAvailable = m_fh->length() - offset;
	if (length == _AU_LENGTH_UNSPECIFIED || static_cast<off_t>(length) > lengthAvailable)
		length = lengthAvailable;

	track->data_size = length;

	switch (encoding)
	{
		case _AU_FORMAT_MULAW_8:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_G711_ULAW;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.bytesPerPacket = track->f.channelCount;
			break;
		case _AU_FORMAT_ALAW_8:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_G711_ALAW;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.bytesPerPacket = track->f.channelCount;
			break;
		case _AU_FORMAT_LINEAR_8:
			track->f.sampleWidth = 8;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_LINEAR_16:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_LINEAR_24:
			track->f.sampleWidth = 24;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_LINEAR_32:
			track->f.sampleWidth = 32;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_FLOAT:
			track->f.sampleWidth = 32;
			track->f.sampleFormat = AF_SAMPFMT_FLOAT;
			break;
		case _AU_FORMAT_DOUBLE:
			track->f.sampleWidth = 64;
			track->f.sampleFormat = AF_SAMPFMT_DOUBLE;
			break;

		default:
			/*
				This encoding method is not recognized.
			*/
			_af_error(AF_BAD_SAMPFMT, "bad sample format");
			return AF_FAIL;
	}

	if (track->f.isUncompressed())
		track->f.computeBytesPerPacketPCM();

	_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth);

	track->computeTotalFileFrames();

	return AF_SUCCEED;
}

bool NeXTFile::recognize(File *fh)
{
	uint8_t buffer[4];

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(buffer, 4) != 4 || memcmp(buffer, ".snd", 4) != 0)
		return false;

	return true;
}

AFfilesetup NeXTFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "NeXT files must have exactly 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	if (track->f.sampleFormat == AF_SAMPFMT_UNSIGNED)
	{
		_af_error(AF_BAD_FILEFMT, "NeXT format does not support unsigned data");
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, track->f.sampleWidth);
	}

	if (track->f.sampleFormat == AF_SAMPFMT_TWOSCOMP)
	{
		if (track->f.sampleWidth != 8 &&
			track->f.sampleWidth != 16 &&
			track->f.sampleWidth != 24 &&
			track->f.sampleWidth != 32)
		{
			_af_error(AF_BAD_WIDTH, "invalid sample width %d for NeXT file (only 8-, 16-, 24-, and 32-bit data are allowed)", track->f.sampleWidth);
			return AF_NULL_FILESETUP;
		}
	}

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "compression format not implemented for NeXT files");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed() &&
		track->byteOrderSet &&
		track->f.byteOrder != AF_BYTEORDER_BIGENDIAN &&
		track->f.isByteOrderSignificant())
	{
		_af_error(AF_BAD_BYTEORDER, "NeXT format supports only big-endian data");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed())
		track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT files cannot have AES data");
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT format does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT format does not support instruments");
		return AF_NULL_FILESETUP;
	}

	if (setup->miscellaneousSet && setup->miscellaneousCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT format does not support miscellaneous data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &nextDefaultFileSetup, false);
}

static uint32_t nextencodingtype (AudioFormat *format);

status NeXTFile::update()
{
	writeHeader();
	return AF_SUCCEED;
}

status NeXTFile::writeHeader()
{
	Track *track = getTrack();

	if (m_fh->seek(0, File::SeekFromBeginning) != 0)
		_af_error(AF_BAD_LSEEK, "bad seek");

	uint32_t offset = track->fpos_first_frame;
	uint32_t length = track->data_size;
	uint32_t encoding = nextencodingtype(&track->f);
	uint32_t sampleRate = track->f.sampleRate;
	uint32_t channelCount = track->f.channelCount;

	m_fh->write(".snd", 4);
	writeU32(&offset);
	writeU32(&length);
	writeU32(&encoding);
	writeU32(&sampleRate);
	writeU32(&channelCount);

	return AF_SUCCEED;
}

static uint32_t nextencodingtype (AudioFormat *format)
{
	uint32_t encoding = 0;

	if (format->compressionType != AF_COMPRESSION_NONE)
	{
		if (format->compressionType == AF_COMPRESSION_G711_ULAW)
			encoding = _AU_FORMAT_MULAW_8;
		else if (format->compressionType == AF_COMPRESSION_G711_ALAW)
			encoding = _AU_FORMAT_ALAW_8;
	}
	else if (format->sampleFormat == AF_SAMPFMT_TWOSCOMP)
	{
		if (format->sampleWidth == 8)
			encoding = _AU_FORMAT_LINEAR_8;
		else if (format->sampleWidth == 16)
			encoding = _AU_FORMAT_LINEAR_16;
		else if (format->sampleWidth == 24)
			encoding = _AU_FORMAT_LINEAR_24;
		else if (format->sampleWidth == 32)
			encoding = _AU_FORMAT_LINEAR_32;
	}
	else if (format->sampleFormat == AF_SAMPFMT_FLOAT)
		encoding = _AU_FORMAT_FLOAT;
	else if (format->sampleFormat == AF_SAMPFMT_DOUBLE)
		encoding = _AU_FORMAT_DOUBLE;

	return encoding;
}

status NeXTFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	writeHeader();

	Track *track = getTrack();
	track->fpos_first_frame = 28;

	return AF_SUCCEED;
}
