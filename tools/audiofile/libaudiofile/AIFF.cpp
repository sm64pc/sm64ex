/*
	Audio File Library
	Copyright (C) 1998-2000, 2003-2004, 2010-2013, Michael Pruett <michael@68k.org>
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

/*
	AIFF.cpp

	This file contains routines for reading and writing AIFF and
	AIFF-C sound files.
*/

#include "config.h"
#include "AIFF.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "File.h"
#include "Instrument.h"
#include "Marker.h"
#include "Setup.h"
#include "Tag.h"
#include "Track.h"
#include "byteorder.h"
#include "extended.h"
#include "util.h"

const InstParamInfo _af_aiff_inst_params[_AF_AIFF_NUM_INSTPARAMS] =
{
	{ AF_INST_MIDI_BASENOTE, AU_PVTYPE_LONG, "MIDI base note", {60} },
	{ AF_INST_NUMCENTS_DETUNE, AU_PVTYPE_LONG, "Detune in cents", {0} },
	{ AF_INST_MIDI_LOVELOCITY, AU_PVTYPE_LONG, "Low velocity", {1} },
	{ AF_INST_MIDI_HIVELOCITY, AU_PVTYPE_LONG, "High velocity", {127} },
	{ AF_INST_MIDI_LONOTE, AU_PVTYPE_LONG, "Low note", {0} },
	{ AF_INST_MIDI_HINOTE, AU_PVTYPE_LONG, "High note", {127} },
	{ AF_INST_NUMDBS_GAIN, AU_PVTYPE_LONG, "Gain in dB", {0} },
	{ AF_INST_SUSLOOPID, AU_PVTYPE_LONG, "Sustain loop id", {0} },
	{ AF_INST_RELLOOPID, AU_PVTYPE_LONG, "Release loop id", {0} }
};

const int _af_aiffc_compression_types[_AF_AIFFC_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW,
	AF_COMPRESSION_IMA
};

static const _AFfilesetup aiffDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_AIFF,		/* fileFormat */
	true,			/* trackSet */
	true,			/* instrumentSet */
	true,			/* miscellaneousSet */
	1,			/* trackCount */
	NULL,			/* tracks */
	1,			/* instrumentCount */
	NULL,			/* instruments */
	0,			/* miscellaneousCount */
	NULL			/* miscellaneous */
};

#define AIFC_VERSION_1 0xa2805140

struct _INST
{
	uint8_t		baseNote;
	int8_t		detune;
	uint8_t		lowNote, highNote;
	uint8_t		lowVelocity, highVelocity;
	int16_t		gain;

	uint16_t	sustainLoopPlayMode;
	uint16_t	sustainLoopBegin;
	uint16_t	sustainLoopEnd;

	uint16_t	releaseLoopPlayMode;
	uint16_t	releaseLoopBegin;
	uint16_t	releaseLoopEnd;
};

AIFFFile::AIFFFile()
{
	setFormatByteOrder(AF_BYTEORDER_BIGENDIAN);

	m_miscellaneousPosition = 0;
	m_FVER_offset = 0;
	m_COMM_offset = 0;
	m_MARK_offset = 0;
	m_INST_offset = 0;
	m_AESD_offset = 0;
	m_SSND_offset = 0;
}

/*
	FVER chunks are only present in AIFF-C files.
*/
status AIFFFile::parseFVER(const Tag &type, size_t size)
{
	assert(type == "FVER");

	uint32_t timestamp;
	readU32(&timestamp);
	/* timestamp holds the number of seconds since January 1, 1904. */

	return AF_SUCCEED;
}

/*
	Parse AES recording data.
*/
status AIFFFile::parseAESD(const Tag &type, size_t size)
{
	unsigned char aesChannelStatusData[24];

	assert(type == "AESD");
	assert(size == 24);

	Track *track = getTrack();

	track->hasAESData = true;

	/*
		Try to read 24 bytes of AES nonaudio data from the file.
		Fail if the file disappoints.
	*/
	if (m_fh->read(aesChannelStatusData, 24) != 24)
		return AF_FAIL;

	memcpy(track->aesData, aesChannelStatusData, 24);

	return AF_SUCCEED;
}

/*
	Parse miscellaneous data chunks such as name, author, copyright,
	and annotation chunks.
*/
status AIFFFile::parseMiscellaneous(const Tag &type, size_t size)
{
	int misctype = AF_MISC_UNRECOGNIZED;

	assert(type == "NAME" ||
		type == "AUTH" ||
		type == "(c) " ||
		type == "ANNO" ||
		type == "APPL" ||
		type == "MIDI");

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
	else if (type == "APPL")
		misctype = AF_MISC_APPL;
	else if (type == "MIDI")
		misctype = AF_MISC_MIDI;

	m_miscellaneous[m_miscellaneousCount - 1].id = m_miscellaneousCount;
	m_miscellaneous[m_miscellaneousCount - 1].type = misctype;
	m_miscellaneous[m_miscellaneousCount - 1].size = size;
	m_miscellaneous[m_miscellaneousCount - 1].position = 0;
	m_miscellaneous[m_miscellaneousCount - 1].buffer = _af_malloc(size);
	m_fh->read(m_miscellaneous[m_miscellaneousCount - 1].buffer, size);

	return AF_SUCCEED;
}

/*
	Parse instrument chunks, which contain information about using
	sound data as a sampled instrument.
*/
status AIFFFile::parseINST(const Tag &type, size_t size)
{
	uint8_t baseNote;
	int8_t detune;
	uint8_t lowNote, highNote, lowVelocity, highVelocity;
	int16_t gain;

	uint16_t sustainLoopPlayMode, sustainLoopBegin, sustainLoopEnd;
	uint16_t releaseLoopPlayMode, releaseLoopBegin, releaseLoopEnd;

	Instrument *instrument = (Instrument *) _af_calloc(1, sizeof (Instrument));
	instrument->id = AF_DEFAULT_INST;
	instrument->values = (AFPVu *) _af_calloc(_AF_AIFF_NUM_INSTPARAMS, sizeof (AFPVu));
	instrument->loopCount = 2;
	instrument->loops = (Loop *) _af_calloc(2, sizeof (Loop));

	m_instrumentCount = 1;
	m_instruments = instrument;

	readU8(&baseNote);
	readS8(&detune);
	readU8(&lowNote);
	readU8(&highNote);
	readU8(&lowVelocity);
	readU8(&highVelocity);
	readS16(&gain);

	instrument->values[0].l = baseNote;
	instrument->values[1].l = detune;
	instrument->values[2].l = lowVelocity;
	instrument->values[3].l = highVelocity;
	instrument->values[4].l = lowNote;
	instrument->values[5].l = highNote;
	instrument->values[6].l = gain;

	instrument->values[7].l = 1;	/* sustain loop id */
	instrument->values[8].l = 2;	/* release loop id */

	readU16(&sustainLoopPlayMode);
	readU16(&sustainLoopBegin);
	readU16(&sustainLoopEnd);

	readU16(&releaseLoopPlayMode);
	readU16(&releaseLoopBegin);
	readU16(&releaseLoopEnd);

	instrument->loops[0].id = 1;
	instrument->loops[0].mode = sustainLoopPlayMode;
	instrument->loops[0].beginMarker = sustainLoopBegin;
	instrument->loops[0].endMarker = sustainLoopEnd;
	instrument->loops[0].trackid = AF_DEFAULT_TRACK;

	instrument->loops[1].id = 2;
	instrument->loops[1].mode = releaseLoopPlayMode;
	instrument->loops[1].beginMarker = releaseLoopBegin;
	instrument->loops[1].endMarker = releaseLoopEnd;
	instrument->loops[1].trackid = AF_DEFAULT_TRACK;

	return AF_SUCCEED;
}

/*
	Parse marker chunks, which contain the positions and names of loop markers.
*/
status AIFFFile::parseMARK(const Tag &type, size_t size)
{
	assert(type == "MARK");

	Track *track = getTrack();

	uint16_t numMarkers;
	readU16(&numMarkers);

	track->markerCount = numMarkers;
	if (numMarkers)
		track->markers = _af_marker_new(numMarkers);

	for (unsigned i=0; i<numMarkers; i++)
	{
		uint16_t markerID = 0;
		uint32_t markerPosition = 0;
		uint8_t sizeByte = 0;
		char *markerName = NULL;

		readU16(&markerID);
		readU32(&markerPosition);
		m_fh->read(&sizeByte, 1);
		markerName = (char *) _af_malloc(sizeByte + 1);
		m_fh->read(markerName, sizeByte);

		markerName[sizeByte] = '\0';

		/*
			If sizeByte is even, then 1+sizeByte (the length
			of the string) is odd.	Skip an extra byte to
			make it even.
		*/

		if ((sizeByte % 2) == 0)
			m_fh->seek(1, File::SeekFromCurrent);

		track->markers[i].id = markerID;
		track->markers[i].position = markerPosition;
		track->markers[i].name = markerName;
		track->markers[i].comment = _af_strdup("");
	}

	return AF_SUCCEED;
}

/*
	Parse common data chunks, which contain information regarding the
	sampling rate, the number of sample frames, and the number of
	sound channels.
*/
status AIFFFile::parseCOMM(const Tag &type, size_t size)
{
	assert(type == "COMM");

	Track *track = getTrack();

	uint16_t numChannels;
	uint32_t numSampleFrames;
	uint16_t sampleSize;
	unsigned char sampleRate[10];

	readU16(&numChannels);
	track->f.channelCount = numChannels;

	if (!numChannels)
	{
		_af_error(AF_BAD_CHANNELS, "invalid file with 0 channels");
		return AF_FAIL;
	}

	readU32(&numSampleFrames);
	track->totalfframes = numSampleFrames;

	readU16(&sampleSize);
	track->f.sampleWidth = sampleSize;

	m_fh->read(sampleRate, 10);
	track->f.sampleRate = _af_convert_from_ieee_extended(sampleRate);

	track->f.compressionType = AF_COMPRESSION_NONE;
	track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
	track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;

	track->f.framesPerPacket = 1;

	if (isAIFFC())
	{
		Tag compressionID;
		// Pascal strings are at most 255 bytes long.
		char compressionName[256];

		readTag(&compressionID);

		// Read the Pascal-style string containing the name.
		readPString(compressionName);

		if (compressionID == "NONE" || compressionID == "twos")
		{
			track->f.compressionType = AF_COMPRESSION_NONE;
		}
		else if (compressionID == "in24")
		{
			track->f.compressionType = AF_COMPRESSION_NONE;
			track->f.sampleWidth = 24;
		}
		else if (compressionID == "in32")
		{
			track->f.compressionType = AF_COMPRESSION_NONE;
			track->f.sampleWidth = 32;
		}
		else if (compressionID == "ACE2" ||
			compressionID == "ACE8" ||
			compressionID == "MAC3" ||
			compressionID == "MAC6")
		{
			_af_error(AF_BAD_NOT_IMPLEMENTED, "AIFF-C format does not support Apple's proprietary %s compression format", compressionName);
			return AF_FAIL;
		}
		else if (compressionID == "ulaw" || compressionID == "ULAW")
		{
			track->f.compressionType = AF_COMPRESSION_G711_ULAW;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.sampleWidth = 16;
			track->f.bytesPerPacket = track->f.channelCount;
		}
		else if (compressionID == "alaw" || compressionID == "ALAW")
		{
			track->f.compressionType = AF_COMPRESSION_G711_ALAW;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.sampleWidth = 16;
			track->f.bytesPerPacket = track->f.channelCount;
		}
		else if (compressionID == "fl32" || compressionID == "FL32")
		{
			track->f.sampleFormat = AF_SAMPFMT_FLOAT;
			track->f.sampleWidth = 32;
			track->f.compressionType = AF_COMPRESSION_NONE;
		}
		else if (compressionID == "fl64" || compressionID == "FL64")
		{
			track->f.sampleFormat = AF_SAMPFMT_DOUBLE;
			track->f.sampleWidth = 64;
			track->f.compressionType = AF_COMPRESSION_NONE;
		}
		else if (compressionID == "sowt")
		{
			track->f.compressionType = AF_COMPRESSION_NONE;
			track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
		}
		else if (compressionID == "ima4")
		{
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_IMA;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;

			initIMACompressionParams();

			track->totalfframes *= 64;
		}
		else
		{
			_af_error(AF_BAD_NOT_IMPLEMENTED, "AIFF-C compression type '%s' not currently supported",
				compressionID.name().c_str());
			return AF_FAIL;
		}
	}

	if (track->f.isUncompressed())
		track->f.computeBytesPerPacketPCM();

	if (_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth) == AF_FAIL)
		return AF_FAIL;

	return AF_SUCCEED;
}

/*
	Parse the stored sound chunk, which usually contains little more
	than the sound data.
*/
status AIFFFile::parseSSND(const Tag &type, size_t size)
{
	assert(type == "SSND");

	Track *track = getTrack();

	uint32_t offset, blockSize;
	readU32(&offset);
	readU32(&blockSize);

	track->data_size = size - 8 - offset;

	track->fpos_first_frame = m_fh->tell() + offset;

	return AF_SUCCEED;
}

status AIFFFile::readInit(AFfilesetup setup)
{
	uint32_t type, size, formtype;

	bool hasCOMM = false;
	bool hasFVER = false;
	bool hasSSND = false;

	m_fh->seek(0, File::SeekFromBeginning);

	m_fh->read(&type, 4);
	readU32(&size);
	m_fh->read(&formtype, 4);

	if (memcmp(&type, "FORM", 4) != 0 ||
		(memcmp(&formtype, "AIFF", 4) && memcmp(&formtype, "AIFC", 4)))
		return AF_FAIL;

	if (!allocateTrack())
		return AF_FAIL;

	/* Include the offset of the form type. */
	size_t index = 4;
	while (index < size)
	{
		Tag chunkid;
		uint32_t chunksize = 0;
		status result = AF_SUCCEED;

		readTag(&chunkid);
		readU32(&chunksize);

		if (chunkid == "COMM")
		{
			hasCOMM = true;
			result = parseCOMM(chunkid, chunksize);
		}
		else if (chunkid == "FVER")
		{
			hasFVER = true;
			parseFVER(chunkid, chunksize);
		}
		else if (chunkid == "INST")
		{
			parseINST(chunkid, chunksize);
		}
		else if (chunkid == "MARK")
		{
			parseMARK(chunkid, chunksize);
		}
		else if (chunkid == "AESD")
		{
			parseAESD(chunkid, chunksize);
		}
		else if (chunkid == "NAME" ||
			chunkid == "AUTH" ||
			chunkid == "(c) " ||
			chunkid == "ANNO" ||
			chunkid == "APPL" ||
			chunkid == "MIDI")
		{
			parseMiscellaneous(chunkid, chunksize);
		}
		/*
			The sound data chunk is required if there are more than
			zero sample frames.
		*/
		else if (chunkid == "SSND")
		{
			if (hasSSND)
			{
				_af_error(AF_BAD_AIFF_SSND, "AIFF file has more than one SSND chunk");
				return AF_FAIL;
			}
			hasSSND = true;
			result = parseSSND(chunkid, chunksize);
		}

		if (result == AF_FAIL)
			return AF_FAIL;

		index += chunksize + 8;

		/* all chunks must be aligned on an even number of bytes */
		if ((index % 2) != 0)
			index++;

		m_fh->seek(index + 8, File::SeekFromBeginning);
	}

	if (!hasCOMM)
	{
		_af_error(AF_BAD_AIFF_COMM, "bad AIFF COMM chunk");
	}

	if (isAIFFC() && !hasFVER)
	{
		_af_error(AF_BAD_HEADER, "FVER chunk is required in AIFF-C");
	}

	/* The file has been successfully parsed. */
	return AF_SUCCEED;
}

bool AIFFFile::recognizeAIFF(File *fh)
{
	uint8_t buffer[8];

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(buffer, 8) != 8 || memcmp(buffer, "FORM", 4) != 0)
		return false;
	if (fh->read(buffer, 4) != 4 || memcmp(buffer, "AIFF", 4) != 0)
		return false;

	return true;
}

bool AIFFFile::recognizeAIFFC(File *fh)
{
	uint8_t buffer[8];

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(buffer, 8) != 8 || memcmp(buffer, "FORM", 4) != 0)
		return false;
	if (fh->read(buffer, 4) != 4 || memcmp(buffer, "AIFC", 4) != 0)
		return false;

	return true;
}

AFfilesetup AIFFFile::completeSetup(AFfilesetup setup)
{
	bool	isAIFF = setup->fileFormat == AF_FILE_AIFF;

	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "AIFF/AIFF-C file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	if (track->sampleFormatSet)
	{
		if (track->f.sampleFormat == AF_SAMPFMT_UNSIGNED)
		{
			_af_error(AF_BAD_FILEFMT, "AIFF/AIFF-C format does not support unsigned data");
			return AF_NULL_FILESETUP;
		}
		else if (isAIFF && track->f.sampleFormat != AF_SAMPFMT_TWOSCOMP)
		{
			_af_error(AF_BAD_FILEFMT, "AIFF format supports only two's complement integer data");
			return AF_NULL_FILESETUP;
		}
	}
	else
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP,
			track->f.sampleWidth);

	/* Check sample width if writing two's complement. Otherwise ignore. */
	if (track->f.sampleFormat == AF_SAMPFMT_TWOSCOMP &&
		(track->f.sampleWidth < 1 || track->f.sampleWidth > 32))
	{
		_af_error(AF_BAD_WIDTH,
			"invalid sample width %d for AIFF/AIFF-C file "
			"(must be 1-32)", track->f.sampleWidth);
		return AF_NULL_FILESETUP;
	}

	if (isAIFF && track->f.compressionType != AF_COMPRESSION_NONE)
	{
		_af_error(AF_BAD_FILESETUP,
			"AIFF does not support compression; use AIFF-C");
		return AF_NULL_FILESETUP;
	}

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW &&
		track->f.compressionType != AF_COMPRESSION_IMA)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "compression format not supported in AIFF-C");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed() &&
		track->byteOrderSet &&
		track->f.byteOrder != AF_BYTEORDER_BIGENDIAN &&
		track->f.isByteOrderSignificant())
	{
		_af_error(AF_BAD_BYTEORDER,
			"AIFF/AIFF-C format supports only big-endian data");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed())
		track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;

	if (setup->instrumentSet)
	{
		if (setup->instrumentCount != 0 && setup->instrumentCount != 1)
		{
			_af_error(AF_BAD_NUMINSTS, "AIFF/AIFF-C file must have 0 or 1 instrument chunk");
			return AF_NULL_FILESETUP;
		}
		if (setup->instruments != 0 &&
			setup->instruments[0].loopCount != 2)
		{
			_af_error(AF_BAD_NUMLOOPS, "AIFF/AIFF-C file with instrument must also have 2 loops");
			return AF_NULL_FILESETUP;
		}
	}

	if (setup->miscellaneousSet)
	{
		for (int i=0; i<setup->miscellaneousCount; i++)
		{
			switch (setup->miscellaneous[i].type)
			{
				case AF_MISC_COPY:
				case AF_MISC_AUTH:
				case AF_MISC_NAME:
				case AF_MISC_ANNO:
				case AF_MISC_APPL:
				case AF_MISC_MIDI:
					break;

				default:
					_af_error(AF_BAD_MISCTYPE, "invalid miscellaneous type %d for AIFF/AIFF-C file", setup->miscellaneous[i].type);
					return AF_NULL_FILESETUP;
			}
		}
	}

	return _af_filesetup_copy(setup, &aiffDefaultFileSetup, true);
}

bool AIFFFile::isInstrumentParameterValid(AUpvlist list, int i)
{
	int param, type;

	AUpvgetparam(list, i, &param);
	AUpvgetvaltype(list, i, &type);
	if (type != AU_PVTYPE_LONG)
		return false;

	long lval;
	AUpvgetval(list, i, &lval);

	switch (param)
	{
		case AF_INST_MIDI_BASENOTE:
			return ((lval >= 0) && (lval <= 127));

		case AF_INST_NUMCENTS_DETUNE:
			return ((lval >= -50) && (lval <= 50));

		case AF_INST_MIDI_LOVELOCITY:
			return ((lval >= 1) && (lval <= 127));

		case AF_INST_MIDI_HIVELOCITY:
			return ((lval >= 1) && (lval <= 127));

		case AF_INST_MIDI_LONOTE:
			return ((lval >= 0) && (lval <= 127));

		case AF_INST_MIDI_HINOTE:
			return ((lval >= 0) && (lval <= 127));

		case AF_INST_NUMDBS_GAIN:
		case AF_INST_SUSLOOPID:
		case AF_INST_RELLOOPID:
			return true;

		default:
			return false;
			break;
	}

	return true;
}

int AIFFFile::getVersion()
{
	if (isAIFFC())
		return AIFC_VERSION_1;
	return 0;
}

status AIFFFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	initCompressionParams();

	uint32_t fileSize = 0;
	m_fh->write("FORM", 4);
	writeU32(&fileSize);

	if (isAIFFC())
		m_fh->write("AIFC", 4);
	else
		m_fh->write("AIFF", 4);

	if (isAIFFC())
		writeFVER();

	writeCOMM();
	writeMARK();
	writeINST();
	writeAESD();
	writeMiscellaneous();
	writeSSND();

	return AF_SUCCEED;
}

status AIFFFile::update()
{
	/* Get the length of the file. */
	uint32_t length = m_fh->length();
	length -= 8;

	/* Set the length of the FORM chunk. */
	m_fh->seek(4, File::SeekFromBeginning);
	writeU32(&length);

	if (isAIFFC())
		writeFVER();

	writeCOMM();
	writeMARK();
	writeINST();
	writeAESD();
	writeMiscellaneous();
	writeSSND();

	return AF_SUCCEED;
}

status AIFFFile::writeCOMM()
{
	/*
		If COMM_offset hasn't been set yet, set it to the
		current offset.
	*/
	if (m_COMM_offset == 0)
		m_COMM_offset = m_fh->tell();
	else
		m_fh->seek(m_COMM_offset, File::SeekFromBeginning);

	Track *track = getTrack();

	Tag compressionTag;
	/* Pascal strings can occupy only 255 bytes (+ a size byte). */
	char compressionName[256];

	if (isAIFFC())
	{
		if (track->f.compressionType == AF_COMPRESSION_NONE)
		{
			if (track->f.sampleFormat == AF_SAMPFMT_TWOSCOMP)
			{
				compressionTag = "NONE";
				strcpy(compressionName, "not compressed");
			}
			else if (track->f.sampleFormat == AF_SAMPFMT_FLOAT)
			{
				compressionTag = "fl32";
				strcpy(compressionName, "32-bit Floating Point");
			}
			else if (track->f.sampleFormat == AF_SAMPFMT_DOUBLE)
			{
				compressionTag = "fl64";
				strcpy(compressionName, "64-bit Floating Point");
			}
			/*
				We disallow unsigned sample data for
				AIFF files in _af_aiff_complete_setup,
				so the next condition should never be
				satisfied.
			*/
			else if (track->f.sampleFormat == AF_SAMPFMT_UNSIGNED)
			{
				_af_error(AF_BAD_SAMPFMT,
					"AIFF/AIFF-C format does not support unsigned data");
				assert(0);
				return AF_FAIL;
			}
		}
		else if (track->f.compressionType == AF_COMPRESSION_G711_ULAW)
		{
			compressionTag = "ulaw";
			strcpy(compressionName, "CCITT G.711 u-law");
		}
		else if (track->f.compressionType == AF_COMPRESSION_G711_ALAW)
		{
			compressionTag = "alaw";
			strcpy(compressionName, "CCITT G.711 A-law");
		}
		else if (track->f.compressionType == AF_COMPRESSION_IMA)
		{
			compressionTag = "ima4";
			strcpy(compressionName, "IMA 4:1 compression");
		}
	}

	m_fh->write("COMM", 4);

	/*
		For AIFF-C files, the length of the COMM chunk is 22
		plus the length of the compression name plus the size
		byte.  If the length of the data is an odd number of
		bytes, add a zero pad byte at the end, but don't
		include the pad byte in the chunk's size.
	*/
	uint32_t chunkSize;
	if (isAIFFC())
		chunkSize = 22 + strlen(compressionName) + 1;
	else
		chunkSize = 18;
	writeU32(&chunkSize);

	/* number of channels, 2 bytes */
	uint16_t channelCount = track->f.channelCount;
	writeU16(&channelCount);

	/* number of sample frames, 4 bytes */
	uint32_t frameCount = track->totalfframes;
	if (track->f.compressionType == AF_COMPRESSION_IMA)
		frameCount = track->totalfframes / track->f.framesPerPacket;
	writeU32(&frameCount);

	/* sample size, 2 bytes */
	uint16_t sampleSize = track->f.sampleWidth;
	writeU16(&sampleSize);

	/* sample rate, 10 bytes */
	uint8_t sampleRate[10];
	_af_convert_to_ieee_extended(track->f.sampleRate, sampleRate);
	m_fh->write(sampleRate, 10);

	if (isAIFFC())
	{
		writeTag(&compressionTag);
		writePString(compressionName);
	}

	return AF_SUCCEED;
}

/*
	The AESD chunk contains information pertinent to audio recording
	devices.
*/
status AIFFFile::writeAESD()
{
	Track *track = getTrack();

	if (!track->hasAESData)
		return AF_SUCCEED;

	if (m_AESD_offset == 0)
		m_AESD_offset = m_fh->tell();
	else
		m_fh->seek(m_AESD_offset, File::SeekFromBeginning);

	if (m_fh->write("AESD", 4) < 4)
		return AF_FAIL;

	uint32_t size = 24;
	if (!writeU32(&size))
		return AF_FAIL;

	if (m_fh->write(track->aesData, 24) < 24)
		return AF_FAIL;

	return AF_SUCCEED;
}

status AIFFFile::writeSSND()
{
	Track *track = getTrack();

	if (m_SSND_offset == 0)
		m_SSND_offset = m_fh->tell();
	else
		m_fh->seek(m_SSND_offset, File::SeekFromBeginning);

	m_fh->write("SSND", 4);

	uint32_t chunkSize = track->data_size + 8;
	writeU32(&chunkSize);

	uint32_t zero = 0;
	/* data offset */
	writeU32(&zero);
	/* block size */
	writeU32(&zero);

	if (track->fpos_first_frame == 0)
		track->fpos_first_frame = m_fh->tell();

	return AF_SUCCEED;
}

status AIFFFile::writeINST()
{
	uint32_t length = 20;

	struct _INST instrumentdata;

	instrumentdata.sustainLoopPlayMode =
		afGetLoopMode(this, AF_DEFAULT_INST, 1);
	instrumentdata.sustainLoopBegin =
		afGetLoopStart(this, AF_DEFAULT_INST, 1);
	instrumentdata.sustainLoopEnd =
		afGetLoopEnd(this, AF_DEFAULT_INST, 1);

	instrumentdata.releaseLoopPlayMode =
		afGetLoopMode(this, AF_DEFAULT_INST, 2);
	instrumentdata.releaseLoopBegin =
		afGetLoopStart(this, AF_DEFAULT_INST, 2);
	instrumentdata.releaseLoopEnd =
		afGetLoopEnd(this, AF_DEFAULT_INST, 2);

	m_fh->write("INST", 4);
	writeU32(&length);

	instrumentdata.baseNote =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_MIDI_BASENOTE);
	writeU8(&instrumentdata.baseNote);
	instrumentdata.detune =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_NUMCENTS_DETUNE);
	writeS8(&instrumentdata.detune);
	instrumentdata.lowNote =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_MIDI_LONOTE);
	writeU8(&instrumentdata.lowNote);
	instrumentdata.highNote =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_MIDI_HINOTE);
	writeU8(&instrumentdata.highNote);
	instrumentdata.lowVelocity =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_MIDI_LOVELOCITY);
	writeU8(&instrumentdata.lowVelocity);
	instrumentdata.highVelocity =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_MIDI_HIVELOCITY);
	writeU8(&instrumentdata.highVelocity);

	instrumentdata.gain =
		afGetInstParamLong(this, AF_DEFAULT_INST, AF_INST_NUMDBS_GAIN);
	writeS16(&instrumentdata.gain);

	writeU16(&instrumentdata.sustainLoopPlayMode);
	writeU16(&instrumentdata.sustainLoopBegin);
	writeU16(&instrumentdata.sustainLoopEnd);

	writeU16(&instrumentdata.releaseLoopPlayMode);
	writeU16(&instrumentdata.releaseLoopBegin);
	writeU16(&instrumentdata.releaseLoopEnd);

	return AF_SUCCEED;
}

status AIFFFile::writeMARK()
{
	Track *track = getTrack();
	if (!track->markerCount)
		return AF_SUCCEED;

	if (m_MARK_offset == 0)
		m_MARK_offset = m_fh->tell();
	else
		m_fh->seek(m_MARK_offset, File::SeekFromBeginning);

	Tag markTag("MARK");
	uint32_t length = 0;

	writeTag(&markTag);
	writeU32(&length);

	AFfileoffset chunkStartPosition = m_fh->tell();

	uint16_t numMarkers = track->markerCount;
	writeU16(&numMarkers);

	for (unsigned i=0; i<numMarkers; i++)
	{
		uint16_t id = track->markers[i].id;
		writeU16(&id);

		uint32_t position = track->markers[i].position;
		writeU32(&position);

		const char *name = track->markers[i].name;
		assert(name);

		// Write the name as a Pascal-style string.
		writePString(name);
	}

	AFfileoffset chunkEndPosition = m_fh->tell();
	length = chunkEndPosition - chunkStartPosition;

	m_fh->seek(chunkStartPosition - 4, File::SeekFromBeginning);

	writeU32(&length);
	m_fh->seek(chunkEndPosition, File::SeekFromBeginning);

	return AF_SUCCEED;
}

/*
	The FVER chunk, if present, is always the first chunk in the file.
*/
status AIFFFile::writeFVER()
{
	uint32_t chunkSize, timeStamp;

	assert(isAIFFC());

	if (m_FVER_offset == 0)
		m_FVER_offset = m_fh->tell();
	else
		m_fh->seek(m_FVER_offset, File::SeekFromBeginning);

	m_fh->write("FVER", 4);

	chunkSize = 4;
	writeU32(&chunkSize);

	timeStamp = AIFC_VERSION_1;
	writeU32(&timeStamp);

	return AF_SUCCEED;
}

/*
	WriteMiscellaneous writes all the miscellaneous data chunks in a
	file handle structure to an AIFF or AIFF-C file.
*/
status AIFFFile::writeMiscellaneous()
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
			case AF_MISC_MIDI:
				chunkType = "MIDI"; break;
			case AF_MISC_APPL:
				chunkType = "APPL"; break;
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

void AIFFFile::initCompressionParams()
{
	Track *track = getTrack();
	if (track->f.compressionType == AF_COMPRESSION_IMA)
		initIMACompressionParams();
}

void AIFFFile::initIMACompressionParams()
{
	Track *track = getTrack();

	track->f.bytesPerPacket = 34 * track->f.channelCount;
	track->f.framesPerPacket = 64;

	AUpvlist pv = AUpvnew(1);
	AUpvsetparam(pv, 0, _AF_IMA_ADPCM_TYPE);
	AUpvsetvaltype(pv, 0, AU_PVTYPE_LONG);
	long l = _AF_IMA_ADPCM_TYPE_QT;
	AUpvsetval(pv, 0, &l);

	track->f.compressionParams = pv;
}

// Read a Pascal-style string.
bool AIFFFile::readPString(char s[256])
{
	uint8_t length;
	if (m_fh->read(&length, 1) != 1)
		return false;
	if (m_fh->read(s, length) != static_cast<ssize_t>(length))
		return false;
	s[length] = '\0';
	return true;
}

// Write a Pascal-style string.
bool AIFFFile::writePString(const char *s)
{
	size_t length = strlen(s);
	if (length > 255)
		return false;
	uint8_t sizeByte = static_cast<uint8_t>(length);
	if (m_fh->write(&sizeByte, 1) != 1)
		return false;
	if (m_fh->write(s, length) != (ssize_t) length)
		return false;
	/*
		Add a pad byte if the length of the Pascal-style string
		(including the size byte) is odd.
	*/
	if ((length % 2) == 0)
	{
		uint8_t zero = 0;
		if (m_fh->write(&zero, 1) != 1)
			return false;
	}
	return true;
}
