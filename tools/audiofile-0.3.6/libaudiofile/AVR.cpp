/*
	Audio File Library
	Copyright (C) 2004, Michael Pruett <michael@68k.org>

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
	AVR.cpp

	This file contains routines for reading and writing AVR (Audio
	Visual Research) sound files.
*/

#include "config.h"
#include "AVR.h"

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

static const _AFfilesetup avrDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_AVR,		/* fileFormat */
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

AVRFile::AVRFile()
{
	setFormatByteOrder(AF_BYTEORDER_BIGENDIAN);
}

bool AVRFile::recognize(File *fh)
{
	uint32_t	magic;

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(&magic, 4) != 4 || memcmp(&magic, "2BIT", 4) != 0)
		return false;

	return true;
}

status AVRFile::readInit(AFfilesetup setup)
{
	uint32_t	magic;
	char		name[8];
	uint16_t	mono, resolution, sign, loop, midi;
	uint32_t	rate, size, loopStart, loopEnd;
	char		reserved[26];
	char		user[64];

	m_fh->seek(0, File::SeekFromBeginning);

	if (m_fh->read(&magic, 4) != 4)
	{
		_af_error(AF_BAD_READ, "could not read AVR file header");
		return AF_FAIL;
	}

	if (memcmp(&magic, "2BIT", 4) != 0)
	{
		_af_error(AF_BAD_FILEFMT, "file is not AVR format");
		return AF_FAIL;
	}

	/* Read name. */
	m_fh->read(name, 8);

	readU16(&mono);
	readU16(&resolution);
	readU16(&sign);
	readU16(&loop);
	readU16(&midi);

	readU32(&rate);
	readU32(&size);
	readU32(&loopStart);
	readU32(&loopEnd);

	m_fh->read(reserved, 26);
	m_fh->read(user, 64);

	Track *track = allocateTrack();
	if (!track)
		return AF_FAIL;

	/* Use only low-order three bytes of sample rate. */
	track->f.sampleRate = rate & 0xffffff;

	if (sign == 0)
		track->f.sampleFormat = AF_SAMPFMT_UNSIGNED;
	else if (sign == 0xffff)
		track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
	else
	{
		_af_error(AF_BAD_SAMPFMT, "bad sample format in AVR file");
		return AF_FAIL;
	}

	if (resolution != 8 && resolution != 16)
	{
		_af_error(AF_BAD_WIDTH, "bad sample width %d in AVR file",
			resolution);
		return AF_FAIL;
	}
	track->f.sampleWidth = resolution;

	track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;

	if (mono == 0)
		track->f.channelCount = 1;
	else if (mono == 0xffff)
		track->f.channelCount = 2;
	else
	{
		_af_error(AF_BAD_CHANNELS,
			"invalid number of channels in AVR file");
		return AF_FAIL;
	}

	track->f.compressionType = AF_COMPRESSION_NONE;

	track->f.framesPerPacket = 1;
	track->f.computeBytesPerPacketPCM();

	_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth);

	track->fpos_first_frame = m_fh->tell();
	track->totalfframes = size;
	track->data_size = track->totalfframes * track->f.bytesPerFrame(false);
	track->nextfframe = 0;
	track->fpos_next_frame = track->fpos_first_frame;

	/* The file has been parsed successfully. */
	return AF_SUCCEED;
}

AFfilesetup AVRFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "AVR files must have exactly 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	/* AVR allows only unsigned and two's complement integer data. */
	if (track->f.sampleFormat != AF_SAMPFMT_UNSIGNED &&
		track->f.sampleFormat != AF_SAMPFMT_TWOSCOMP)
	{
		_af_error(AF_BAD_FILEFMT, "AVR format does supports only unsigned and two's complement integer data");
		return AF_NULL_FILESETUP;
	}

	/* For now we support only 8- and 16-bit samples. */
	if (track->f.sampleWidth != 8 && track->f.sampleWidth != 16)
	{
		_af_error(AF_BAD_WIDTH, "invalid sample width %d for AVR file (only 8- and 16-bit sample widths are allowed)", track->f.sampleWidth);
		return AF_NULL_FILESETUP;
	}

	/* AVR does not support compression. */
	if (track->f.compressionType != AF_COMPRESSION_NONE)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "compression not supported for AVR files");
		return AF_NULL_FILESETUP;
	}

	/* AVR audio data is big-endian. */
	if (track->f.byteOrder != AF_BYTEORDER_BIGENDIAN)
	{
		if (track->byteOrderSet)
		{
			_af_error(AF_BAD_BYTEORDER,
				"AVR format supports only big-endian data");
			return AF_NULL_FILESETUP;
		}
		else
			track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "AVR files do not support AES data");
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "AVR format does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "AVR format does not support instruments");
		return AF_NULL_FILESETUP;
	}

	if (setup->miscellaneousSet && setup->miscellaneousCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "AVR format does not support miscellaneous data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &avrDefaultFileSetup, false);
}

status AVRFile::update()
{
	uint32_t	size, loopStart, loopEnd;

	Track *track = getTrack();

	/* Seek to the position of the size field. */
	m_fh->seek(26, File::SeekFromBeginning);

	size = track->totalfframes;

	/* For the case of no loops, loopStart = 0 and loopEnd = size. */
	loopStart = 0;
	loopEnd = size;

	writeU32(&size);
	writeU32(&loopStart);
	writeU32(&loopEnd);

	return AF_SUCCEED;
}

static char *af_basename (char *filename)
{
	char	*base;
	base = strrchr(filename, '/');
	if (base == NULL)
		return filename;
	else
		return base + 1;
}

status AVRFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	if (m_fh->seek(0, File::SeekFromBeginning) != 0)
	{
		_af_error(AF_BAD_LSEEK, "bad seek");
		return AF_FAIL;
	}

	Track *track = getTrack();

	char name[8];
	uint16_t mono, resolution, sign, loop, midi;
	uint32_t rate, size, loopStart, loopEnd;
	char reserved[26];
	char user[64];

	m_fh->write("2BIT", 4);
	memset(name, 0, 8);
	if (m_fileName != NULL)
		strncpy(name, af_basename(m_fileName), 8);
	m_fh->write(name, 8);

	if (track->f.channelCount == 1)
		mono = 0x0;
	else
		mono = 0xffff;
	writeU16(&mono);

	resolution = track->f.sampleWidth;
	writeU16(&resolution);

	if (track->f.sampleFormat == AF_SAMPFMT_UNSIGNED)
		sign = 0x0;
	else
		sign = 0xffff;
	writeU16(&sign);

	/* We do not currently support loops. */
	loop = 0;
	writeU16(&loop);
	midi = 0xffff;
	writeU16(&midi);

	rate = track->f.sampleRate;
	/* Set the high-order byte of rate to 0xff. */
	rate |= 0xff000000;
	size = track->totalfframes;
	loopStart = 0;
	loopEnd = size;

	writeU32(&rate);
	writeU32(&size);
	writeU32(&loopStart);
	writeU32(&loopEnd);

	memset(reserved, 0, 26);
	m_fh->write(reserved, 26);

	memset(user, 0, 64);
	m_fh->write(user, 64);

	if (track->fpos_first_frame == 0)
		track->fpos_first_frame = m_fh->tell();

	return AF_SUCCEED;
}
