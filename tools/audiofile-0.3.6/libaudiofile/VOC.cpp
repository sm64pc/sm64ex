/*
	Audio File Library
	Copyright (C) 2011-2013, Michael Pruett <michael@68k.org>

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
#include "VOC.h"

#include "File.h"
#include "Track.h"
#include "Setup.h"
#include "byteorder.h"
#include "util.h"

#include <string.h>
#include <assert.h>

const int _af_voc_compression_types[_AF_VOC_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW
};

static const int kVOCMagicLength = 20;
static const char kVOCMagic[kVOCMagicLength + 1] = "Creative Voice File\x1a";

enum
{
	kVOCTerminator = 0,
	kVOCSoundData = 1,
	kVOCSoundDataContinuation = 2,
	kVOCSilence = 3,
	kVOCMarker = 4,
	kVOCText = 5,
	kVOCRepeatStart = 6,
	kVOCRepeatEnd = 7,
	kVOCExtendedInfo = 8,
	kVOCSoundDataNew = 9
};

enum
{
	kVOCFormatU8 = 0,
	kVOCFormatCreativeADPCM4_8 = 1,
	kVOCFormatCreativeADPCM3_8 = 2,
	kVOCFormatCreativeADPCM2_8 = 3,
	kVOCFormatS16 = 4,
	kVOCFormatAlaw = 6,
	kVOCFormatUlaw = 7,
	kVOCFormatCreativeADPCM4_16 = 0x200,
};

static const _AFfilesetup vocDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	// valid
	AF_FILE_VOC,			// fileFormat
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

VOCFile::VOCFile() :
	m_soundDataOffset(-1)
{
	setFormatByteOrder(AF_BYTEORDER_LITTLEENDIAN);
}

bool VOCFile::recognize(File *f)
{
	f->seek(0, File::SeekFromBeginning);
	char buffer[kVOCMagicLength];
	if (f->read(buffer, kVOCMagicLength) != kVOCMagicLength)
		return false;
	if (memcmp(buffer, kVOCMagic, kVOCMagicLength) != 0)
		return false;
	return true;
}

AFfilesetup VOCFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "VOC file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = &setup->tracks[0];
	if (track->sampleFormatSet)
	{
		if (!track->f.isInteger())
		{
			_af_error(AF_BAD_SAMPFMT,
				"VOC format supports only integer audio data");
			return AF_NULL_FILESETUP;
		}

		if ((track->f.isSigned() && track->f.sampleWidth != 16) ||
			(track->f.isUnsigned() && track->f.sampleWidth != 8))
		{
			_af_error(AF_BAD_SAMPFMT,
				"VOC format supports only 16-bit signed or 8-bit unsigned data");
			return AF_NULL_FILESETUP;
		}
	}
	else
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP,
			track->f.sampleWidth);

	if (track->f.isUncompressed() &&
		track->byteOrderSet &&
		track->f.byteOrder != AF_BYTEORDER_LITTLEENDIAN &&
		track->f.isByteOrderSignificant())
	{
		_af_error(AF_BAD_BYTEORDER, "VOC supports only little-endian data");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed())
		track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW)
	{
		_af_error(AF_BAD_COMPTYPE,
			"compression format %d not supported in VOC file",
			track->f.compressionType);
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount)
	{
		_af_error(AF_BAD_NUMMARKS, "VOC does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "VOC does not support AES data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &vocDefaultFileSetup, true);
}

status VOCFile::readInit(AFfilesetup)
{
	m_fh->seek(20, File::SeekFromBeginning);

	uint16_t dataOffset, version, checksum;
	readU16(&dataOffset);
	readU16(&version);
	readU16(&checksum);

	Track *track = allocateTrack();

	bool hasExtendedInfo = false;
	bool foundSoundData = false;

	off_t position = m_fh->tell();
	off_t fileLength = m_fh->length();
	while (position < fileLength)
	{
		uint32_t blockHeader;
		if (!readU32(&blockHeader))
			break;
		uint8_t blockType = blockHeader & 0xff;
		uint32_t blockSize = blockHeader >> 8;

		if (blockType == kVOCSoundData)
		{
			if (foundSoundData)
			{
				_af_error(AF_BAD_HEADER, "VOC file contains multiple sound data blocks");
				return AF_FAIL;
			}

			foundSoundData = true;

			uint8_t frequencyDivisor, codec;
			readU8(&frequencyDivisor);
			readU8(&codec);

			if (!hasExtendedInfo)
			{
				track->f.channelCount = 1;
				track->f.sampleRate = 1000000 / (256 - frequencyDivisor);
			}

			track->f.compressionType = AF_COMPRESSION_NONE;
			track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
			track->f.framesPerPacket = 1;

			if (codec == kVOCFormatU8)
			{
				_af_set_sample_format(&track->f, AF_SAMPFMT_UNSIGNED, 8);
				track->f.computeBytesPerPacketPCM();
			}
			else if (codec == kVOCFormatCreativeADPCM4_8 ||
				codec == kVOCFormatCreativeADPCM3_8 ||
				codec == kVOCFormatCreativeADPCM2_8)
			{
				_af_error(AF_BAD_NOT_IMPLEMENTED,
					"Creative ADPCM compression is not currently suppported");
				return AF_FAIL;
			}
			else
			{
				_af_error(AF_BAD_CODEC_TYPE,
					"VOC file contains unrecognized codec type %d", codec);
				return AF_FAIL;
			}

			track->fpos_first_frame = m_fh->tell();
			track->data_size = m_fh->length() - 1 - track->fpos_first_frame;
			track->computeTotalFileFrames();
		}
		else if (blockType == kVOCExtendedInfo)
		{
			if (foundSoundData)
			{
				_af_error(AF_BAD_HEADER, "VOC extended information found after sound data");
				return AF_FAIL;
			}

			hasExtendedInfo = true;

			uint16_t frequencyDivisor;
			uint8_t bitsPerSample;
			uint8_t isStereo;
			readU16(&frequencyDivisor);
			readU8(&bitsPerSample);
			readU8(&isStereo);

			track->f.sampleWidth = bitsPerSample;
			track->f.channelCount = isStereo ? 2 : 1;
			uint32_t frequencyDividend = 256000000 / (isStereo ? 2 : 1);
			track->f.sampleRate = frequencyDividend / (65536 - frequencyDivisor);
		}
		else if (blockType == kVOCSoundDataNew)
		{
			if (foundSoundData)
			{
				_af_error(AF_BAD_HEADER, "VOC file contains multiple sound data blocks");
				return AF_FAIL;
			}

			foundSoundData = true;

			uint32_t sampleRate;
			uint8_t bitsPerSample, channels;
			uint16_t format;
			uint32_t pad;
			readU32(&sampleRate);
			readU8(&bitsPerSample);
			readU8(&channels);
			readU16(&format);
			readU32(&pad);

			if (!channels)
			{
				_af_error(AF_BAD_CHANNELS, "invalid file with 0 channels");
				return AF_FAIL;
			}

			track->fpos_first_frame = m_fh->tell();
			track->data_size = blockSize - 12;

			track->f.compressionType = AF_COMPRESSION_NONE;
			track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
			track->f.sampleRate = sampleRate;
			track->f.channelCount = channels;
			track->f.framesPerPacket = 1;

			if (format == kVOCFormatU8)
			{
				_af_set_sample_format(&track->f, AF_SAMPFMT_UNSIGNED, 8);
				track->f.computeBytesPerPacketPCM();
			}
			else if (format == kVOCFormatS16)
			{
				_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
				track->f.computeBytesPerPacketPCM();
			}
			else if (format == kVOCFormatAlaw)
			{
				track->f.compressionType = AF_COMPRESSION_G711_ALAW;
				track->f.byteOrder = _AF_BYTEORDER_NATIVE;
				track->f.bytesPerPacket = track->f.channelCount;
				_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
			}
			else if (format == kVOCFormatUlaw)
			{
				track->f.compressionType = AF_COMPRESSION_G711_ULAW;
				track->f.byteOrder = _AF_BYTEORDER_NATIVE;
				track->f.bytesPerPacket = track->f.channelCount;
				_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
			}
			else if (format == kVOCFormatCreativeADPCM4_8 ||
				format == kVOCFormatCreativeADPCM3_8 ||
				format == kVOCFormatCreativeADPCM2_8 ||
				format == kVOCFormatCreativeADPCM4_16)
			{
				_af_error(AF_BAD_NOT_IMPLEMENTED,
					"Creative ADPCM compression is not currently supported");
				return AF_FAIL;
			}
			else
			{
				_af_error(AF_BAD_CODEC_TYPE,
					"VOC file contains unrecognized codec type %d", format);
				return AF_FAIL;
			}

			track->computeTotalFileFrames();
		}

		position += 4 + blockSize;

		m_fh->seek(position, File::SeekFromBeginning);
	}

	return AF_SUCCEED;
}

status VOCFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	m_fh->write(kVOCMagic, kVOCMagicLength);
	uint16_t dataOffset = 0x001a;
	uint16_t version = 0x0114;
	uint16_t checksum = 0x1234 + ~version;
	writeU16(&dataOffset);
	writeU16(&version);
	writeU16(&checksum);

	return writeSoundData();
}

status VOCFile::update()
{
	if (writeSoundData() == AF_FAIL || writeTerminator() == AF_FAIL)
		return AF_FAIL;
	return AF_SUCCEED;
}

status VOCFile::writeSoundData()
{
	if (m_soundDataOffset == -1)
		m_soundDataOffset = m_fh->tell();
	else
		m_fh->seek(m_soundDataOffset, File::SeekFromBeginning);

	Track *track = getTrack();

	assert((track->f.isSigned() && track->f.sampleWidth == 16) ||
		(track->f.isUnsigned() && track->f.sampleWidth == 8));

	uint8_t blockType = kVOCSoundDataNew;
	uint32_t blockSize = 12 + track->data_size;
	uint32_t blockHeader = blockSize << 8 | blockType;
	if (!writeU32(&blockHeader))
		return AF_FAIL;

	uint32_t sampleRate = track->f.sampleRate;
	uint8_t bitsPerSample = track->f.sampleWidth;
	uint8_t channels = track->f.channelCount;
	uint16_t format;
	if (track->f.compressionType == AF_COMPRESSION_G711_ULAW)
	{
		format = kVOCFormatUlaw;
		bitsPerSample = 8;
	}
	else if (track->f.compressionType == AF_COMPRESSION_G711_ALAW)
	{
		format = kVOCFormatAlaw;
		bitsPerSample = 8;
	}
	else if (track->f.compressionType == AF_COMPRESSION_NONE)
	{
		if (track->f.isUnsigned())
			format = kVOCFormatU8;
		else
			format = kVOCFormatS16;
	}
	uint32_t pad = 0;
	if (!writeU32(&sampleRate) ||
		!writeU8(&bitsPerSample) ||
		!writeU8(&channels) ||
		!writeU16(&format) ||
		!writeU32(&pad))
		return AF_FAIL;

	if (track->fpos_first_frame == 0)
		track->fpos_first_frame = m_fh->tell();

	return AF_SUCCEED;
}

status VOCFile::writeTerminator()
{
	Track *track = getTrack();
	m_fh->seek(track->fpos_first_frame + track->data_size, File::SeekFromBeginning);
	uint8_t terminator = kVOCTerminator;
	if (!writeU8(&terminator))
		return AF_FAIL;
	return AF_SUCCEED;
}
