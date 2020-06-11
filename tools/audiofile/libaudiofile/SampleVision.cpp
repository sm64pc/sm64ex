/*
	Audio File Library
	Copyright (C) 2012, Michael Pruett <michael@68k.org>

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
#include "SampleVision.h"

#include "File.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "util.h"
#include <string.h>

static const char *kSMPMagic = "SOUND SAMPLE DATA ";
static const unsigned kSMPMagicLength = 18;
static const char *kSMPVersion = "2.1 ";
static const unsigned kSMPVersionLength = 4;
static const unsigned kSMPNameLength = 30;
static const unsigned kSMPCommentLength = 60;
static const unsigned kSMPMarkerNameLength = 10;
static const uint32_t kSMPInvalidSamplePosition = 0xffffffffu;

static const uint8_t kSMPMIDIUnityPlaybackNote = 60;

static const _AFfilesetup sSampleVisionDefaultFileSetup =
{
	_AF_VALID_FILESETUP,
	AF_FILE_SAMPLEVISION,
	true,	// trackSet
	true,	// instrumentSet
	true,	// miscellaneousSet
	1,		// trackCount
	NULL,	// tracks
	1,		// instrumentCount
	NULL,	// instruments
	0,		// miscellaneousCount
	NULL	// miscellaneous
};

static void trimTrailingSpaces(char *s)
{
	int n = strlen(s);
	if (!n)
		return;
	while (--n > 0 && s[n] == ' ')
		;
	s[n+1] = '\0';
}

SampleVisionFile::SampleVisionFile() :
	m_frameCountOffset(-1)
{
	setFormatByteOrder(AF_BYTEORDER_LITTLEENDIAN);
}

SampleVisionFile::~SampleVisionFile()
{
}

bool SampleVisionFile::recognize(File *fh)
{
	fh->seek(0, File::SeekFromBeginning);
	char magic[kSMPMagicLength];
	if (fh->read(magic, kSMPMagicLength) != (ssize_t) kSMPMagicLength)
		return false;
	return !strncmp(magic, kSMPMagic, kSMPMagicLength);
}

AFfilesetup SampleVisionFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "SampleVision file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	if (track->sampleFormatSet)
	{
		if (!track->f.isSigned() || track->f.sampleWidth != 16)
		{
			_af_error(AF_BAD_SAMPFMT,
				"SampleVision format supports only 16-bit signed integer audio data");
			return AF_NULL_FILESETUP;
		}
	}
	else
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP,
			track->f.sampleWidth);

	if (track->byteOrderSet && track->f.byteOrder != AF_BYTEORDER_LITTLEENDIAN)
	{
		// Treat error as correctable.
		_af_error(AF_BAD_BYTEORDER, "SampleVision supports only little-endian data");
	}

	track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;

	if (track->compressionSet && !track->f.isUncompressed())
	{
		_af_error(AF_BAD_COMPTYPE, "SampleVision does not support compressed audio data");
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount)
	{
		_af_error(AF_BAD_NUMMARKS, "SampleVision does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "SampleVision does not support AES data");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount)
	{
		_af_error(AF_BAD_FILESETUP, "SampleVision does not support instruments");
		return AF_NULL_FILESETUP;
	}

	if (setup->miscellaneousSet && setup->miscellaneousCount)
	{
		_af_error(AF_BAD_FILESETUP, "SampleVision does not support miscellaneous data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &sSampleVisionDefaultFileSetup, true);
}

status SampleVisionFile::readInit(AFfilesetup)
{
	m_fh->seek(0, File::SeekFromBeginning);

	char magic[kSMPMagicLength];
	if (m_fh->read(magic, kSMPMagicLength) != (ssize_t) kSMPMagicLength)
		return AF_FAIL;
	if (strncmp(magic, kSMPMagic, kSMPMagicLength) != 0)
		return AF_FAIL;

	char version[kSMPVersionLength];
	if (m_fh->read(version, kSMPVersionLength) != (ssize_t) kSMPVersionLength)
		return AF_FAIL;
	if (strncmp(version, kSMPVersion, kSMPVersionLength) != 0)
		return AF_FAIL;

	Track *track = allocateTrack();

	char name[kSMPNameLength + 1];
	m_fh->read(name, kSMPNameLength);
	name[kSMPNameLength] = '\0';
	trimTrailingSpaces(name);
	if (strlen(name) > 0)
		addMiscellaneous(AF_MISC_NAME, name);

	char comment[kSMPCommentLength + 1];
	m_fh->read(comment, kSMPCommentLength);
	comment[kSMPCommentLength] = '\0';
	trimTrailingSpaces(comment);
	if (strlen(comment) > 0)
		addMiscellaneous(AF_MISC_COMMENT, comment);

	uint32_t frameCount;
	readU32(&frameCount);
	track->totalfframes = frameCount;
	track->fpos_first_frame = m_fh->tell();
	track->data_size = 2 * frameCount;

	m_fh->seek(track->data_size, File::SeekFromCurrent);

	uint16_t reserved;
	readU16(&reserved);

	parseLoops();
	parseMarkers();

	uint8_t midiNote;
	uint32_t sampleRate;
	uint32_t smpteOffset;
	uint32_t cycleLength;

	readU8(&midiNote);
	readU32(&sampleRate);
	readU32(&smpteOffset);
	readU32(&cycleLength);

	track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
	track->f.sampleRate = sampleRate;
	track->f.channelCount = 1;
	track->f.compressionType = AF_COMPRESSION_NONE;
	track->f.framesPerPacket = 1;
	_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
	track->f.computeBytesPerPacketPCM();

	return AF_SUCCEED;
}

status SampleVisionFile::parseLoops()
{
	for (int i=0; i<8; i++)
	{
		uint32_t startFrame, endFrame;
		uint8_t type;
		uint16_t count;
		readU32(&startFrame);
		readU32(&endFrame);
		readU8(&type);
		readU16(&count);
	}
	return AF_SUCCEED;
}

status SampleVisionFile::parseMarkers()
{
	for (int i=0; i<8; i++)
	{
		char name[kSMPMarkerNameLength + 1];
		m_fh->read(name, kSMPMarkerNameLength);
		name[kSMPMarkerNameLength] = '\0';
		uint32_t position;
		readU32(&position);
	}
	return AF_SUCCEED;
}

void SampleVisionFile::addMiscellaneous(int type, const char *data)
{
	m_miscellaneousCount++;
	m_miscellaneous = (Miscellaneous *) _af_realloc(m_miscellaneous,
		m_miscellaneousCount * sizeof (Miscellaneous));

	Miscellaneous &m = m_miscellaneous[m_miscellaneousCount - 1];
	m.id = m_miscellaneousCount;
	m.type = type;
	m.size = strlen(data);
	m.position = 0;
	m.buffer = _af_malloc(m.size);
	memcpy(m.buffer, data, m.size);
}

status SampleVisionFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	m_fh->write(kSMPMagic, kSMPMagicLength);
	m_fh->write(kSMPVersion, kSMPVersionLength);

	char name[kSMPNameLength + 1];
	char comment[kSMPCommentLength + 1];
	memset(name, ' ', kSMPNameLength);
	memset(comment, ' ', kSMPCommentLength);
	m_fh->write(name, kSMPNameLength);
	m_fh->write(comment, kSMPCommentLength);

	uint32_t frameCount = 0;
	m_frameCountOffset = m_fh->tell();
	writeU32(&frameCount);

	Track *track = getTrack();
	track->fpos_first_frame = m_fh->tell();

	return AF_SUCCEED;
}

status SampleVisionFile::update()
{
	m_fh->seek(m_frameCountOffset, File::SeekFromBeginning);
	Track *track = getTrack();
	uint32_t frameCount = track->totalfframes;
	writeU32(&frameCount);
	writeTrailer();
	return AF_SUCCEED;
}

status SampleVisionFile::writeTrailer()
{
	Track *track = getTrack();

	m_fh->seek(track->fpos_after_data, File::SeekFromBeginning);

	uint16_t reserved = 0;
	writeU16(&reserved);

	writeLoops();
	writeMarkers();

	uint8_t midiNote = kSMPMIDIUnityPlaybackNote;
	uint32_t sampleRate = track->f.sampleRate;
	uint32_t smpteOffset = 0;
	uint32_t cycleLength = 0;

	writeU8(&midiNote);
	writeU32(&sampleRate);
	writeU32(&smpteOffset);
	writeU32(&cycleLength);
	return AF_SUCCEED;
}

status SampleVisionFile::writeLoops()
{
	for (int i=0; i<8; i++)
	{
		uint32_t startFrame = kSMPInvalidSamplePosition, endFrame = 0;
		uint8_t type = 0;
		uint16_t count = 0;
		writeU32(&startFrame);
		writeU32(&endFrame);
		writeU8(&type);
		writeU16(&count);
	}
	return AF_SUCCEED;
}

status SampleVisionFile::writeMarkers()
{
	for (int i=0; i<8; i++)
	{
		char name[kSMPMarkerNameLength + 1];
		memset(name, ' ', kSMPMarkerNameLength);
		name[kSMPMarkerNameLength] = '\0';
		m_fh->write(name, kSMPMarkerNameLength);
		uint32_t position = kSMPInvalidSamplePosition;
		writeU32(&position);
	}
	return AF_SUCCEED;
}
