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
	IFF.cpp

	This file contains routines for reading and writing IFF/8SVX
	sound files.
*/

#include "config.h"
#include "IFF.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "File.h"
#include "Marker.h"
#include "Setup.h"
#include "Tag.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "byteorder.h"
#include "util.h"

static const _AFfilesetup iffDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_IFF_8SVX,	/* fileFormat */
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

bool IFFFile::recognize(File *fh)
{
	uint8_t buffer[8];

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(buffer, 8) != 8 || memcmp(buffer, "FORM", 4) != 0)
		return false;
	if (fh->read(buffer, 4) != 4 || memcmp(buffer, "8SVX", 4) != 0)
		return false;

	return true;
}

IFFFile::IFFFile()
{
	setFormatByteOrder(AF_BYTEORDER_BIGENDIAN);

	m_miscellaneousPosition = 0;
	m_VHDR_offset = 0;
	m_BODY_offset = 0;
}

/*
	Parse miscellaneous data chunks such as name, author, copyright,
	and annotation chunks.
*/
status IFFFile::parseMiscellaneous(const Tag &type, size_t size)
{
	int misctype = AF_MISC_UNRECOGNIZED;

	assert(type == "NAME" || type == "AUTH" ||
		type == "(c) " || type == "ANNO");

	/* Skip zero-length miscellaneous chunks. */
	if (size == 0)
		return AF_FAIL;

	m_miscellaneousCount++;
	m_miscellaneous = (Miscellaneous *) _af_realloc(m_miscellaneous,
		m_miscellaneousCount * sizeof (Miscellaneous));

	if (type == "NAME")
		misctype = AF_MISC_NAME;
	else if (type == "AUTH")
		misctype = AF_MISC_AUTH;
	else if (type == "(c) ")
		misctype = AF_MISC_COPY;
	else if (type == "ANNO")
		misctype = AF_MISC_ANNO;

	m_miscellaneous[m_miscellaneousCount - 1].id = m_miscellaneousCount;
	m_miscellaneous[m_miscellaneousCount - 1].type = misctype;
	m_miscellaneous[m_miscellaneousCount - 1].size = size;
	m_miscellaneous[m_miscellaneousCount - 1].position = 0;
	m_miscellaneous[m_miscellaneousCount - 1].buffer = _af_malloc(size);
	m_fh->read(m_miscellaneous[m_miscellaneousCount - 1].buffer, size);

	return AF_SUCCEED;
}

/*
	Parse voice header chunk.
*/
status IFFFile::parseVHDR(const Tag &type, size_t size)
{
	assert(type == "VHDR");

	Track *track = getTrack();

	uint32_t oneShotSamples, repeatSamples, samplesPerRepeat;
	uint16_t sampleRate;
	uint8_t octaves, compression;
	uint32_t volume;

	readU32(&oneShotSamples);
	readU32(&repeatSamples);
	readU32(&samplesPerRepeat);
	readU16(&sampleRate);
	readU8(&octaves);
	readU8(&compression);
	readU32(&volume);

	track->f.sampleWidth = 8; 
	track->f.sampleRate = sampleRate;
	track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
	track->f.compressionType = AF_COMPRESSION_NONE;
	track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;
	track->f.channelCount = 1;

	track->f.framesPerPacket = 1;
	track->f.computeBytesPerPacketPCM();

	_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth);

	return AF_SUCCEED;
}

status IFFFile::parseBODY(const Tag &type, size_t size)
{
	Track *track = getTrack();

	/*
		IFF/8SVX files have only one audio channel with one
		byte per sample, so the number of frames is equal to
		the number of bytes.
	*/
	track->totalfframes = size;
	track->data_size = size;

	/* Sound data follows. */
	track->fpos_first_frame = m_fh->tell();

	return AF_SUCCEED;
}

status IFFFile::readInit(AFfilesetup setup)
{
	m_fh->seek(0, File::SeekFromBeginning);

	Tag type;
	uint32_t size;
	Tag formtype;

	readTag(&type);
	readU32(&size);
	readTag(&formtype);

	if (type != "FORM" || formtype != "8SVX")
		return AF_FAIL;

	/* IFF/8SVX files have only one track. */
	Track *track = allocateTrack();
	if (!track)
		return AF_FAIL;

	/* Set the index to include the form type ('8SVX' in this case). */
	size_t index = 4;
	while (index < size)
	{
		Tag chunkid;
		uint32_t chunksize = 0;
		status result = AF_SUCCEED;

		readTag(&chunkid);
		readU32(&chunksize);

		if (chunkid == "VHDR")
		{
			result = parseVHDR(chunkid, chunksize);
		}
		else if (chunkid == "BODY")
		{
			result = parseBODY(chunkid, chunksize);
		}
		else if (chunkid == "NAME" ||
			chunkid == "AUTH" ||
			chunkid == "(c) " ||
			chunkid == "ANNO")
		{
			parseMiscellaneous(chunkid, chunksize);
		}

		if (result == AF_FAIL)
			return AF_FAIL;

		/*
			Increment the index by the size of the chunk
			plus the size of the chunk header.
		*/
		index += chunksize + 8;

		/* All chunks must be aligned on an even number of bytes. */
		if ((index % 2) != 0)
			index++;

		/* Set the seek position to the beginning of the next chunk. */
		m_fh->seek(index + 8, File::SeekFromBeginning);
	}

	/* The file has been successfully parsed. */
	return AF_SUCCEED;
}

AFfilesetup IFFFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "IFF/8SVX file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	if (track->sampleFormatSet &&
		track->f.sampleFormat != AF_SAMPFMT_TWOSCOMP)
	{
		_af_error(AF_BAD_SAMPFMT,
			"IFF/8SVX format supports only two's complement integer data");
		return AF_NULL_FILESETUP;
	}

	if (track->sampleFormatSet && track->f.sampleWidth != 8)
	{
		_af_error(AF_BAD_WIDTH,
			"IFF/8SVX file allows only 8 bits per sample "
			"(%d bits requested)", track->f.sampleWidth);
		return AF_NULL_FILESETUP;
	}

	if (track->channelCountSet && track->f.channelCount != 1)
	{
		_af_error(AF_BAD_CHANNELS,
			"invalid channel count (%d) for IFF/8SVX format "
			"(only 1 channel supported)",
			track->f.channelCount);
		return AF_NULL_FILESETUP;
	}

	if (track->f.compressionType != AF_COMPRESSION_NONE)
	{
		_af_error(AF_BAD_COMPRESSION,
			"IFF/8SVX does not support compression");
		return AF_NULL_FILESETUP;
	}

	/* Ignore requested byte order since samples are only one byte. */
	track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;
	/* Either one channel was requested or no request was made. */
	track->f.channelCount = 1;
	_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 8);

	if (track->markersSet && track->markerCount != 0)
	{
		_af_error(AF_BAD_NUMMARKS,
			"IFF/8SVX format does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "IFF/8SVX format does not support AES data");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount != 0)
	{
		_af_error(AF_BAD_NUMINSTS,
			"IFF/8SVX format does not support instruments");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &iffDefaultFileSetup, true);
}

status IFFFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	uint32_t fileSize = 0;

	m_fh->write("FORM", 4);
	writeU32(&fileSize);

	m_fh->write("8SVX", 4);

	writeVHDR();
	writeMiscellaneous();
	writeBODY();

	return AF_SUCCEED;
}

status IFFFile::update()
{
	uint32_t length;

	writeVHDR();
	writeMiscellaneous();
	writeBODY();

	/* Get the length of the file. */
	length = m_fh->length();
	length -= 8;

	/* Set the length of the FORM chunk. */
	m_fh->seek(4, File::SeekFromBeginning);
	writeU32(&length);

	return AF_SUCCEED;
}

status IFFFile::writeVHDR()
{
	uint32_t chunkSize;
	uint32_t oneShotSamples, repeatSamples, samplesPerRepeat;
	uint16_t sampleRate;
	uint8_t octaves, compression;
	uint32_t volume;

	/*
		If VHDR_offset hasn't been set yet, set it to the
		current offset.
	*/
	if (m_VHDR_offset == 0)
		m_VHDR_offset = m_fh->tell();
	else
		m_fh->seek(m_VHDR_offset, File::SeekFromBeginning);

	Track *track = getTrack();

	m_fh->write("VHDR", 4);

	chunkSize = 20;
	writeU32(&chunkSize);

	/*
		IFF/8SVX files have only one audio channel, so the
		number of samples is equal to the number of frames.
	*/
	oneShotSamples = track->totalfframes;
	writeU32(&oneShotSamples);
	repeatSamples = 0;
	writeU32(&repeatSamples);
	samplesPerRepeat = 0;
	writeU32(&samplesPerRepeat);

	sampleRate = track->f.sampleRate;
	writeU16(&sampleRate);

	octaves = 0;
	compression = 0;
	writeU8(&octaves);
	writeU8(&compression);

	/* Volume is in fixed-point notation; 65536 means gain of 1.0. */
	volume = 65536;
	writeU32(&volume);

	return AF_SUCCEED;
}

status IFFFile::writeBODY()
{
	uint32_t chunkSize;

	Track *track = getTrack();

	if (m_BODY_offset == 0)
		m_BODY_offset = m_fh->tell();
	else
		m_fh->seek(m_BODY_offset, File::SeekFromBeginning);

	m_fh->write("BODY", 4);

	/*
		IFF/8SVX supports only one channel, so the number of
		frames is equal to the number of samples, and each
		sample is one byte.
	*/
	chunkSize = track->totalfframes;
	writeU32(&chunkSize);

	if (track->fpos_first_frame == 0)
		track->fpos_first_frame = m_fh->tell();

	/* Add a pad byte to the end of the chunk if the chunk size is odd. */
	if ((chunkSize % 2) == 1)
	{
		uint8_t zero = 0;
		m_fh->seek(m_BODY_offset + 8 + chunkSize, File::SeekFromBeginning);
		writeU8(&zero);
	}

	return AF_SUCCEED;
}

/*
	WriteMiscellaneous writes all the miscellaneous data chunks in a
	file handle structure to an IFF/8SVX file.
*/
status IFFFile::writeMiscellaneous()
{
	if (m_miscellaneousPosition == 0)
		m_miscellaneousPosition = m_fh->tell();
	else
		m_fh->seek(m_miscellaneousPosition, File::SeekFromBeginning);

	for (int i=0; i<m_miscellaneousCount; i++)
	{
		Miscellaneous *misc = &m_miscellaneous[i];
		Tag chunkType;
		uint32_t chunkSize;
		uint8_t padByte = 0;

		switch (misc->type)
		{
			case AF_MISC_NAME:
				chunkType = "NAME"; break;
			case AF_MISC_AUTH:
				chunkType = "AUTH"; break;
			case AF_MISC_COPY:
				chunkType = "(c) "; break;
			case AF_MISC_ANNO:
				chunkType = "ANNO"; break;
		}

		writeTag(&chunkType);

		chunkSize = misc->size;
		writeU32(&chunkSize);

		/*
			Write the miscellaneous buffer and then a pad byte
			if necessary.  If the buffer is null, skip the space
			for now.
		*/
		if (misc->buffer != NULL)
			m_fh->write(misc->buffer, misc->size);
		else
			m_fh->seek(misc->size, File::SeekFromCurrent);

		if (misc->size % 2 != 0)
			writeU8(&padByte);
	}

	return AF_SUCCEED;
}
