/*
	Audio File Library
	Copyright (C) 2001, Silicon Graphics, Inc.
	Copyright (C) 2011, Michael Pruett <michael@68k.org>

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
	IRCAM.cpp

	This file contains routines for reading and writing
	Berkeley/IRCAM/CARL sound files.
*/

#include "config.h"
#include "IRCAM.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "File.h"
#include "Marker.h"
#include "Setup.h"
#include "Track.h"
#include "byteorder.h"
#include "util.h"

enum
{
	SF_CHAR = 1,
	SF_SHORT = 2,
	SF_24INT = 3,
	SF_LONG = 0x40004,
	SF_FLOAT = 4,
	SF_DOUBLE = 8,
	SF_ALAW = 0x10001,
	SF_ULAW = 0x20001
};

#define SF_MAXCHAN 4
#define SF_MAXCOMMENT 512
#define SF_MINCOMMENT 256

enum
{
	SF_END,
	SF_MAXAMP,
	SF_COMMENT,
	SF_LINKCODE
};

#define SIZEOF_BSD_HEADER 1024

static const uint8_t ircam_vax_le_magic[4] = {0x64, 0xa3, 0x01, 0x00},
	ircam_vax_be_magic[4] = {0x00, 0x01, 0xa3, 0x64},
	ircam_sun_be_magic[4] = {0x64, 0xa3, 0x02, 0x00},
	ircam_sun_le_magic[4] = {0x00, 0x02, 0xa3, 0x64},
	ircam_mips_le_magic[4] = {0x64, 0xa3, 0x03, 0x00},
	ircam_mips_be_magic[4] = {0x00, 0x03, 0xa3, 0x64},
	ircam_next_be_magic[4] = {0x64, 0xa3, 0x04, 0x00},
	ircam_next_le_magic[4] = {0x00, 0x04, 0xa3, 0x64};

static const _AFfilesetup ircam_default_filesetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_IRCAM,		/* fileFormat */
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

const int _af_ircam_compression_types[_AF_IRCAM_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW
};

bool IRCAMFile::recognize(File *fh)
{
	uint8_t buffer[4];

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(buffer, 4) != 4)
		return false;

	/* Check to see if the file's magic number matches. */
	if (!memcmp(buffer, ircam_vax_le_magic, 4) ||
		!memcmp(buffer, ircam_vax_be_magic, 4) ||
		!memcmp(buffer, ircam_sun_be_magic, 4) ||
		!memcmp(buffer, ircam_sun_le_magic, 4) ||
		!memcmp(buffer, ircam_mips_le_magic, 4) ||
		!memcmp(buffer, ircam_mips_be_magic, 4) ||
		!memcmp(buffer, ircam_next_be_magic, 4) ||
		!memcmp(buffer, ircam_next_le_magic, 4))
	{
		return true;
	}

	return false;
}

AFfilesetup IRCAMFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "BICSF file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = &setup->tracks[0];
	if (track->sampleFormatSet)
	{
		if (track->f.isUnsigned())
		{
			_af_error(AF_BAD_SAMPFMT,
				"BICSF format does not support unsigned data");
			return AF_NULL_FILESETUP;
		}

		if (track->f.isSigned() &&
			track->f.sampleWidth != 8 &&
			track->f.sampleWidth != 16 &&
			track->f.sampleWidth != 24 &&
			track->f.sampleWidth != 32)
		{
			_af_error(AF_BAD_WIDTH,
				"BICSF format supports only 8-, 16-, 24-, or 32-bit "
				"two's complement audio data");
			return AF_NULL_FILESETUP;
		}
	}

	if (track->rateSet && track->f.sampleRate <= 0.0)
	{
		_af_error(AF_BAD_RATE,
			"invalid sample rate %.30g for BICSF file",
			track->f.sampleRate);
		return AF_NULL_FILESETUP;
	}

	if (track->channelCountSet && track->f.channelCount != 1 &&
		track->f.channelCount != 2 && track->f.channelCount != 4)
	{
		_af_error(AF_BAD_CHANNELS,
			"invalid channel count (%d) for BICSF format "
			"(1, 2, or 4 channels only)",
			track->f.channelCount);
		return AF_NULL_FILESETUP;
	}

	if (track->compressionSet &&
		track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED,
			"BICSF format does not support compression type %d",
			track->f.compressionType);
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "BICSF file cannot have AES data");
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount != 0)
	{
		_af_error(AF_BAD_NUMMARKS, "BICSF format does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount != 0)
	{
		_af_error(AF_BAD_NUMINSTS, "BICSF format does not support instruments");
		return AF_NULL_FILESETUP;
	}

	/* XXXmpruett: We don't support miscellaneous chunks for now. */
	if (setup->miscellaneousSet && setup->miscellaneousCount != 0)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "BICSF format does not currently support miscellaneous chunks");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &ircam_default_filesetup, true);
}

status IRCAMFile::readInit(AFfilesetup setup)
{
	float maxAmp = 1.0;

	m_fh->seek(0, File::SeekFromBeginning);

	uint8_t magic[4];
	if (m_fh->read(magic, 4) != 4)
	{
		_af_error(AF_BAD_READ, "Could not read BICSF file header");
		return AF_FAIL;
	}

	if (memcmp(magic, ircam_vax_le_magic, 4) != 0 &&
		memcmp(magic, ircam_vax_be_magic, 4) != 0 &&
		memcmp(magic, ircam_sun_be_magic, 4) != 0 &&
		memcmp(magic, ircam_sun_le_magic, 4) != 0 &&
		memcmp(magic, ircam_mips_le_magic, 4) != 0 &&
		memcmp(magic, ircam_mips_be_magic, 4) != 0 &&
		memcmp(magic, ircam_next_be_magic, 4) != 0 &&
		memcmp(magic, ircam_next_le_magic, 4) != 0)
	{
		_af_error(AF_BAD_FILEFMT,
			"file is not a BICSF file (bad magic number)");
		return AF_FAIL;
	}

	// Check whether the file's magic number indicates little-endian data.
	bool isLittleEndian = !memcmp(magic, ircam_vax_le_magic, 4) ||
		!memcmp(magic, ircam_sun_le_magic, 4) ||
		!memcmp(magic, ircam_mips_le_magic, 4) ||
		!memcmp(magic, ircam_next_le_magic, 4);

	setFormatByteOrder(isLittleEndian ? AF_BYTEORDER_LITTLEENDIAN :
		AF_BYTEORDER_BIGENDIAN);

	float rate;
	readFloat(&rate);
	uint32_t channels;
	readU32(&channels);
	uint32_t packMode;
	readU32(&packMode);

	Track *track = allocateTrack();
	if (!track)
		return AF_FAIL;

	track->f.sampleRate = rate;
	track->f.compressionType = AF_COMPRESSION_NONE;

	if (isLittleEndian)
		track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
	else
		track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;

	if (channels != 1 && channels != 2 && channels != 4)
	{
		_af_error(AF_BAD_FILEFMT,
			"invalid channel count (%d) for BICSF format (1, 2, or 4 only)",
			channels);
		return AF_FAIL;
	}

	track->f.channelCount = channels;

	track->f.framesPerPacket = 1;

	switch (packMode)
	{
		case SF_CHAR:
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.sampleWidth = 8;
			break;
		case SF_SHORT:
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.sampleWidth = 16;
			break;
		case SF_24INT:
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.sampleWidth = 24;
			break;
		case SF_LONG:
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.sampleWidth = 32;
			break;
		case SF_FLOAT:
			track->f.sampleFormat = AF_SAMPFMT_FLOAT;
			track->f.sampleWidth = 32;
			break;
		case SF_DOUBLE:
			track->f.sampleFormat = AF_SAMPFMT_DOUBLE;
			track->f.sampleWidth = 64;
			break;
		case SF_ALAW:
			track->f.compressionType = AF_COMPRESSION_G711_ALAW;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.sampleWidth = 16;
			track->f.bytesPerPacket = channels;
			break;
		case SF_ULAW:
			track->f.compressionType = AF_COMPRESSION_G711_ULAW;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.sampleWidth = 16;
			track->f.bytesPerPacket = channels;
			break;
		default:
			_af_error(AF_BAD_NOT_IMPLEMENTED,
				"BICSF data format %d not supported", packMode);
			return AF_FAIL;
	}

	if (track->f.isUncompressed())
		track->f.computeBytesPerPacketPCM();

	if (_af_set_sample_format(&track->f, track->f.sampleFormat,
		track->f.sampleWidth) == AF_FAIL)
	{
		return AF_FAIL;
	}

	if (track->f.sampleFormat == AF_SAMPFMT_FLOAT)
		track->f.pcm.slope = maxAmp;

	track->data_size = m_fh->length() - SIZEOF_BSD_HEADER;
	track->computeTotalFileFrames();

	track->fpos_first_frame = SIZEOF_BSD_HEADER;
	track->nextfframe = 0;
	track->fpos_next_frame = track->fpos_first_frame;

	return AF_SUCCEED;
}

/* We write IRCAM files using the native byte order. */
status IRCAMFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	uint32_t dataOffset = SIZEOF_BSD_HEADER;

	Track *track = getTrack();
	track->totalfframes = 0;
	track->fpos_first_frame = dataOffset;
	track->nextfframe = 0;
	track->fpos_next_frame = track->fpos_first_frame;

	/* Choose the magic number appropriate for the byte order. */
	const uint8_t *magic;
#ifdef WORDS_BIGENDIAN
	magic = ircam_sun_be_magic;
#else
	magic = ircam_vax_le_magic;
#endif

	uint32_t channels = track->f.channelCount;
	float rate = track->f.sampleRate;

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW)
	{
		_af_error(AF_BAD_COMPTYPE,
			"unsupported compression type %d in IRCAM sound file",
			track->f.compressionType);
		return AF_FAIL;
	}

	uint32_t packMode = 0;
	if (track->f.compressionType == AF_COMPRESSION_G711_ULAW)
		packMode = SF_ULAW;
	else if (track->f.compressionType == AF_COMPRESSION_G711_ALAW)
		packMode = SF_ALAW;
	else if (track->f.isSigned())
	{
		switch (track->f.bytesPerSample(false))
		{
			case 1: packMode = SF_CHAR; break;
			case 2: packMode = SF_SHORT; break;
			case 3: packMode = SF_24INT; break;
			case 4: packMode = SF_LONG; break;
			default:
				_af_error(AF_BAD_SAMPFMT,
					"unsupported sample width %d for two's complement BICSF file",
					track->f.sampleWidth);
				return AF_FAIL;
		}
	}
	else if (track->f.isFloat())
	{
		if (track->f.sampleWidth == 32)
			packMode = SF_FLOAT;
		else if (track->f.sampleWidth == 64)
			packMode = SF_DOUBLE;
		else
		{
			_af_error(AF_BAD_SAMPFMT,
				"unsupported sample width %d for BICSF file",
				track->f.sampleWidth);
			return AF_FAIL;
		}
	}
	else if (track->f.isUnsigned())
	{
		_af_error(AF_BAD_SAMPFMT, "BICSF format does not support unsigned integer audio data");
		return AF_FAIL;
	}

	m_fh->seek(0, File::SeekFromBeginning);
	m_fh->write(magic, 4);
	writeFloat(&rate);
	writeU32(&channels);
	writeU32(&packMode);

	/* Zero the entire description block. */
	uint8_t zeros[SIZEOF_BSD_HEADER];
	memset(zeros, 0, SIZEOF_BSD_HEADER);
	m_fh->write(zeros, SIZEOF_BSD_HEADER - 4*4);

	return AF_SUCCEED;
}

status IRCAMFile::update()
{
	return AF_SUCCEED;
}
