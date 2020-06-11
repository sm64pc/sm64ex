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
#include "CAF.h"

#include "Buffer.h"
#include "File.h"
#include "PacketTable.h"
#include "Setup.h"
#include "Tag.h"
#include "Track.h"
#include "byteorder.h"
#include "util.h"

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

const int _af_caf_compression_types[_AF_CAF_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW,
	AF_COMPRESSION_IMA,
	AF_COMPRESSION_ALAC
};

enum
{
	kCAFLinearPCMFormatFlagIsFloat = (1L << 0),
	kCAFLinearPCMFormatFlagIsLittleEndian = (1L << 1)
};

enum
{
	kALACFormatFlag_16BitSourceData = 1,
	kALACFormatFlag_20BitSourceData = 2,
	kALACFormatFlag_24BitSourceData = 3,
	kALACFormatFlag_32BitSourceData = 4
};

static const unsigned kALACDefaultFramesPerPacket = 4096;

static const _AFfilesetup cafDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	// valid
	AF_FILE_CAF,			// fileFormat
	true,					// trackSet
	true,					// instrumentSet
	true,					// miscellaneousSet
	1,						// trackCount
	NULL,					// tracks
	1,						// instrumentCount
	NULL,					// instruments
	0,						// miscellaneousCount
	NULL					// miscellaneous
};

CAFFile::CAFFile() :
	m_dataOffset(-1),
	m_cookieDataOffset(-1)
{
	setFormatByteOrder(AF_BYTEORDER_BIGENDIAN);
}

CAFFile::~CAFFile()
{
}

bool CAFFile::recognize(File *file)
{
	file->seek(0, File::SeekFromBeginning);
	uint8_t buffer[8];
	if (file->read(buffer, 8) != 8 || memcmp(buffer, "caff", 4) != 0)
		return false;
	const uint8_t versionAndFlags[4] = { 0, 1, 0, 0 };
	if (memcmp(buffer + 4, versionAndFlags, 4) != 0)
		return false;
	return true;
}

status CAFFile::readInit(AFfilesetup setup)
{
	m_fh->seek(8, File::SeekFromBeginning);

	if (!allocateTrack())
		return AF_FAIL;

	off_t currentOffset = m_fh->tell();
	off_t fileLength = m_fh->length();

	while (currentOffset < fileLength)
	{
		Tag chunkType;
		int64_t chunkLength;
		if (!readTag(&chunkType) ||
			!readS64(&chunkLength))
			return AF_FAIL;

		currentOffset += 12;

		if (chunkType == "data" && chunkLength == -1)
			chunkLength = fileLength - currentOffset;
		else if (chunkLength < 0)
			_af_error(AF_BAD_HEADER,
				"invalid chunk length %jd for chunk type %s\n",
				static_cast<intmax_t>(chunkLength), chunkType.name().c_str());

		if (chunkType == "desc")
		{
			if (parseDescription(chunkType, chunkLength) == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkType == "data")
		{
			if (parseData(chunkType, chunkLength) == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkType == "pakt")
		{
			if (parsePacketTable(chunkType, chunkLength) == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkType == "kuki")
		{
			if (parseCookieData(chunkType, chunkLength) == AF_FAIL)
				return AF_FAIL;
		}

		currentOffset = m_fh->seek(currentOffset + chunkLength,
			File::SeekFromBeginning);
	}

	return AF_SUCCEED;
}

status CAFFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	initCompressionParams();

	Tag caff("caff");
	if (!writeTag(&caff)) return AF_FAIL;
	const uint8_t versionAndFlags[4] = { 0, 1, 0, 0 };
	if (m_fh->write(versionAndFlags, 4) != 4) return AF_FAIL;

	if (writeDescription() == AF_FAIL)
		return AF_FAIL;
	if (writeCookieData() == AF_FAIL)
		return AF_FAIL;
	if (writeData(false) == AF_FAIL)
		return AF_FAIL;

	return AF_SUCCEED;
}

AFfilesetup CAFFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "CAF file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	if (track->sampleFormatSet)
	{
		if (track->f.isUnsigned())
		{
			_af_error(AF_BAD_FILEFMT, "CAF format does not support unsigned data");
			return AF_NULL_FILESETUP;
		}
	}
	else
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP,
			track->f.sampleWidth);

	if (track->f.isSigned() && (track->f.sampleWidth < 1 || track->f.sampleWidth > 32))
	{
		_af_error(AF_BAD_WIDTH,
			"invalid sample width %d for CAF file (must be 1-32)",
			track->f.sampleWidth);
		return AF_NULL_FILESETUP;
	}

	if (!track->byteOrderSet)
		track->f.byteOrder = _AF_BYTEORDER_NATIVE;

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW &&
		track->f.compressionType != AF_COMPRESSION_IMA &&
		track->f.compressionType != AF_COMPRESSION_ALAC)
	{
		_af_error(AF_BAD_COMPTYPE,
			"compression format %d not supported in CAF file",
			track->f.compressionType);
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "CAF does not yet support markers");
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "CAF does not support AES data");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "CAF does not yet support instruments");
		return AF_NULL_FILESETUP;
	}

	if (setup->miscellaneousSet && setup->miscellaneousCount)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "CAF does not yet support miscellaneous data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &cafDefaultFileSetup, true);
}

status CAFFile::update()
{
	if (writeCookieData() == AF_FAIL)
		return AF_FAIL;
	if (writeData(true) == AF_FAIL)
		return AF_FAIL;
	if (writePacketTable() == AF_FAIL)
		return AF_FAIL;
	return AF_SUCCEED;
}

status CAFFile::parseDescription(const Tag &, int64_t)
{
	double sampleRate;
	Tag formatID;
	uint32_t formatFlags;
	uint32_t bytesPerPacket;
	uint32_t framesPerPacket;
	uint32_t channelsPerFrame;
	uint32_t bitsPerChannel;
	if (!readDouble(&sampleRate) ||
		!readTag(&formatID) ||
		!readU32(&formatFlags) ||
		!readU32(&bytesPerPacket) ||
		!readU32(&framesPerPacket) ||
		!readU32(&channelsPerFrame) ||
		!readU32(&bitsPerChannel))
		return AF_FAIL;

	if (!channelsPerFrame)
	{
		_af_error(AF_BAD_CHANNELS, "invalid file with 0 channels");
		return AF_FAIL;
	}

	Track *track = getTrack();
	track->f.channelCount = channelsPerFrame;
	track->f.sampleWidth = bitsPerChannel;
	track->f.sampleRate = sampleRate;
	track->f.framesPerPacket = 1;

	if (formatID == "lpcm")
	{
		track->f.compressionType = AF_COMPRESSION_NONE;
		if (formatFlags & kCAFLinearPCMFormatFlagIsFloat)
		{
			if (bitsPerChannel != 32 && bitsPerChannel != 64)
			{
				_af_error(AF_BAD_WIDTH, "invalid bits per sample %d for floating-point audio data", bitsPerChannel);
				return AF_FAIL;
			}
			track->f.sampleFormat = bitsPerChannel == 32 ? AF_SAMPFMT_FLOAT :
				AF_SAMPFMT_DOUBLE;
		}
		else
		{
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
		}
		track->f.byteOrder = (formatFlags & kCAFLinearPCMFormatFlagIsLittleEndian) ?
			AF_BYTEORDER_LITTLEENDIAN : AF_BYTEORDER_BIGENDIAN;

		if (_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth) == AF_FAIL)
			return AF_FAIL;

		track->f.computeBytesPerPacketPCM();
		return AF_SUCCEED;
	}
	else if (formatID == "ulaw")
	{
		track->f.compressionType = AF_COMPRESSION_G711_ULAW;
		track->f.byteOrder = _AF_BYTEORDER_NATIVE;
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
		track->f.bytesPerPacket = channelsPerFrame;
		return AF_SUCCEED;
	}
	else if (formatID == "alaw")
	{
		track->f.compressionType = AF_COMPRESSION_G711_ALAW;
		track->f.byteOrder = _AF_BYTEORDER_NATIVE;
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
		track->f.bytesPerPacket = channelsPerFrame;
		return AF_SUCCEED;
	}
	else if (formatID == "ima4")
	{
		track->f.compressionType = AF_COMPRESSION_IMA;
		track->f.byteOrder = _AF_BYTEORDER_NATIVE;
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
		initIMACompressionParams();
		return AF_SUCCEED;
	}
	else if (formatID == "alac")
	{
		track->f.compressionType = AF_COMPRESSION_ALAC;
		track->f.byteOrder = _AF_BYTEORDER_NATIVE;
		switch (formatFlags)
		{
			case kALACFormatFlag_16BitSourceData:
				track->f.sampleWidth = 16; break;
			case kALACFormatFlag_20BitSourceData:
				track->f.sampleWidth = 20; break;
			case kALACFormatFlag_24BitSourceData:
				track->f.sampleWidth = 24; break;
			case kALACFormatFlag_32BitSourceData:
				track->f.sampleWidth = 32; break;
			default:
				_af_error(AF_BAD_CODEC_TYPE,
					"unsupported format flags for ALAC: %u", formatFlags);
				return AF_FAIL;
		}
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP,
			track->f.sampleWidth);
		track->f.framesPerPacket = framesPerPacket;
		track->f.bytesPerPacket = 0;
		return AF_SUCCEED;
	}
	else
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "Compression type %s not supported",
			formatID.name().c_str());
		return AF_FAIL;
	}
}

status CAFFile::parseData(const Tag &tag, int64_t length)
{
	uint32_t editCount;
	if (!readU32(&editCount))
		return AF_FAIL;

	Track *track = getTrack();
	if (length == -1)
		track->data_size = m_fh->length() - m_fh->tell();
	else
		track->data_size = length - 4;
	track->fpos_first_frame = m_fh->tell();

	track->computeTotalFileFrames();
	return AF_SUCCEED;
}

static uint32_t readBERInteger(const uint8_t *input, size_t *numBytes)
{
	uint32_t result = 0;
	uint8_t data;
	size_t size = 0;
	do
	{
		data = input[size];
		result = (result << 7) | (data & 0x7f);
		if (++size > 5)
			return 0;
	} while ((data & 0x80) && size < *numBytes);
	*numBytes = size;
	return result;
}

static void encodeBERInteger(uint32_t value, uint8_t *buffer, size_t *numBytes)
{
	if ((value & 0x7f) == value)
	{
		*numBytes = 1;
		buffer[0] = value;
	}
	else if ((value & 0x3fff) == value)
	{
		*numBytes = 2;
		buffer[0] = (value >> 7) | 0x80;
		buffer[1] = value & 0x7f;
	}
	else if ((value & 0x1fffff) == value)
	{
		*numBytes = 3;
		buffer[0] = (value >> 14) | 0x80;
		buffer[1] = ((value >> 7) & 0x7f) | 0x80;
		buffer[2] = value & 0x7f;
	}
	else if ((value & 0x0fffffff) == value)
	{
		*numBytes = 4;
		buffer[0] = (value >> 21) | 0x80;
		buffer[1] = ((value >> 14) & 0x7f) | 0x80;
		buffer[2] = ((value >> 7) & 0x7f) | 0x80;
		buffer[3] = value & 0x7f;
	}
	else
	{
		*numBytes = 5;
		buffer[0] = (value >> 28) | 0x80;
		buffer[1] = ((value >> 21) & 0x7f) | 0x80;
		buffer[2] = ((value >> 14) & 0x7f) | 0x80;
		buffer[3] = ((value >> 7) & 0x7f) | 0x80;
		buffer[4] = value & 0x7f;
	}
}

status CAFFile::parsePacketTable(const Tag &tag, int64_t length)
{
	if (length < 24)
		return AF_FAIL;

	int64_t numPackets;
	int64_t numValidFrames;
	int32_t primingFrames;
	int32_t remainderFrames;
	if (!readS64(&numPackets) ||
		!readS64(&numValidFrames) ||
		!readS32(&primingFrames) ||
		!readS32(&remainderFrames))
	{
		return AF_FAIL;
	}

	if (!numPackets)
		return AF_SUCCEED;

	int64_t tableLength = length - 24;

	SharedPtr<Buffer> buffer = new Buffer(tableLength);
	if (m_fh->read(buffer->data(), tableLength) != tableLength)
		return AF_FAIL;

	SharedPtr<PacketTable> packetTable = new PacketTable(numValidFrames,
		primingFrames, remainderFrames);

	const uint8_t *data = static_cast<const uint8_t *>(buffer->data());
	size_t position = 0;
	while (position < buffer->size())
	{
		size_t sizeRemaining = buffer->size() - position;
		uint32_t bytesPerPacket = readBERInteger(data + position, &sizeRemaining);
		if (bytesPerPacket == 0)
			break;
		packetTable->append(bytesPerPacket);
		position += sizeRemaining;
	}

	assert(numPackets == packetTable->numPackets());

	Track *track = getTrack();
	track->m_packetTable = packetTable;
	track->totalfframes = numValidFrames;

	return AF_SUCCEED;
}

status CAFFile::parseCookieData(const Tag &tag, int64_t length)
{
	m_codecData = new Buffer(length);
	if (m_fh->read(m_codecData->data(), length) != length)
		return AF_FAIL;

	AUpvlist pv = AUpvnew(2);

	AUpvsetparam(pv, 0, _AF_CODEC_DATA_SIZE);
	AUpvsetvaltype(pv, 0, AU_PVTYPE_LONG);
	long l = length;
	AUpvsetval(pv, 0, &l);

	AUpvsetparam(pv, 1, _AF_CODEC_DATA);
	AUpvsetvaltype(pv, 1, AU_PVTYPE_PTR);
	void *v = m_codecData->data();
	AUpvsetval(pv, 1, &v);

	Track *track = getTrack();
	track->f.compressionParams = pv;

	return AF_SUCCEED;
}

status CAFFile::writeDescription()
{
	Track *track = getTrack();

	Tag desc("desc");
	int64_t chunkLength = 32;
	double sampleRate = track->f.sampleRate;
	Tag formatID("lpcm");
	uint32_t formatFlags = 0;
	if (track->f.byteOrder == AF_BYTEORDER_LITTLEENDIAN)
		formatFlags |= kCAFLinearPCMFormatFlagIsLittleEndian;
	if (track->f.isFloat())
		formatFlags |= kCAFLinearPCMFormatFlagIsFloat;
	uint32_t bytesPerPacket = track->f.bytesPerFrame(false);
	uint32_t framesPerPacket = 1;
	uint32_t channelsPerFrame = track->f.channelCount;
	uint32_t bitsPerChannel = track->f.sampleWidth;

	if (track->f.compressionType == AF_COMPRESSION_G711_ULAW)
	{
		formatID = "ulaw";
		formatFlags = 0;
		bytesPerPacket = channelsPerFrame;
		bitsPerChannel = 8;
	}
	else if (track->f.compressionType == AF_COMPRESSION_G711_ALAW)
	{
		formatID = "alaw";
		formatFlags = 0;
		bytesPerPacket = channelsPerFrame;
		bitsPerChannel = 8;
	}
	else if (track->f.compressionType == AF_COMPRESSION_IMA)
	{
		formatID = "ima4";
		formatFlags = 0;
		bytesPerPacket = track->f.bytesPerPacket;
		framesPerPacket = track->f.framesPerPacket;
		bitsPerChannel = 16;
	}
	else if (track->f.compressionType == AF_COMPRESSION_ALAC)
	{
		formatID = "alac";
		switch (track->f.sampleWidth)
		{
			case 16: formatFlags = kALACFormatFlag_16BitSourceData; break;
			case 20: formatFlags = kALACFormatFlag_20BitSourceData; break;
			case 24: formatFlags = kALACFormatFlag_24BitSourceData; break;
			case 32: formatFlags = kALACFormatFlag_32BitSourceData; break;
		}
		bytesPerPacket = track->f.bytesPerPacket;
		framesPerPacket = track->f.framesPerPacket;
	}

	if (!writeTag(&desc) ||
		!writeS64(&chunkLength) ||
		!writeDouble(&sampleRate) ||
		!writeTag(&formatID) ||
		!writeU32(&formatFlags) ||
		!writeU32(&bytesPerPacket) ||
		!writeU32(&framesPerPacket) ||
		!writeU32(&channelsPerFrame) ||
		!writeU32(&bitsPerChannel))
		return AF_FAIL;
	return AF_SUCCEED;
}

status CAFFile::writeData(bool update)
{
	Track *track = getTrack();

	if (m_dataOffset == -1)
		m_dataOffset = m_fh->tell();
	else
		m_fh->seek(m_dataOffset, File::SeekFromBeginning);

	Tag data("data");
	int64_t dataLength = -1;
	uint32_t editCount = 0;
	if (update)
		dataLength = track->data_size + 4;

	if (!writeTag(&data) ||
		!writeS64(&dataLength) ||
		!writeU32(&editCount))
		return AF_FAIL;
	if (track->fpos_first_frame == 0)
		track->fpos_first_frame = m_fh->tell();
	return AF_SUCCEED;
}

status CAFFile::writePacketTable()
{
	Track *track = getTrack();

	m_fh->seek(track->fpos_after_data, File::SeekFromBeginning);

	SharedPtr<PacketTable> packetTable = track->m_packetTable;
	if (!packetTable)
		return AF_SUCCEED;

	int64_t numPackets = packetTable->numPackets();
	int64_t numValidFrames = packetTable->numValidFrames();
	int32_t primingFrames = packetTable->primingFrames();
	int32_t remainderFrames = packetTable->remainderFrames();

	SharedPtr<Buffer> buffer = new Buffer(packetTable->numPackets() * 5);

	uint8_t *data = static_cast<uint8_t *>(buffer->data());
	size_t position = 0;
	for (unsigned i=0; i<packetTable->numPackets(); i++)
	{
		uint32_t bytesPerPacket = packetTable->bytesPerPacket(i);
		size_t numBytes = 0;
		encodeBERInteger(bytesPerPacket, data + position, &numBytes);
		position += numBytes;
	}

	Tag pakt("pakt");
	int64_t packetTableLength = 24 + position;

	if (!writeTag(&pakt) ||
		!writeS64(&packetTableLength) ||
		!writeS64(&numPackets) ||
		!writeS64(&numValidFrames) ||
		!writeS32(&primingFrames) ||
		!writeS32(&remainderFrames) ||
		m_fh->write(buffer->data(), position) != static_cast<ssize_t>(position))
	{
		return AF_FAIL;
	}

	return AF_SUCCEED;
}

status CAFFile::writeCookieData()
{
	if (!m_codecData)
		return AF_SUCCEED;

	if (m_cookieDataOffset == -1)
		m_cookieDataOffset = m_fh->tell();
	else
		m_fh->seek(m_cookieDataOffset, File::SeekFromBeginning);

	Tag kuki("kuki");
	int64_t cookieDataLength = m_codecData->size();
	if (!writeTag(&kuki) ||
		!writeS64(&cookieDataLength) ||
		m_fh->write(m_codecData->data(), m_codecData->size()) != static_cast<ssize_t>(m_codecData->size()))
	{
		return AF_FAIL;
	}

	return AF_SUCCEED;
}

void CAFFile::initCompressionParams()
{
	Track *track = getTrack();
	if (track->f.compressionType == AF_COMPRESSION_IMA)
		initIMACompressionParams();
	else if (track->f.compressionType == AF_COMPRESSION_ALAC)
		initALACCompressionParams();
}

void CAFFile::initIMACompressionParams()
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

void CAFFile::initALACCompressionParams()
{
	if (m_access == _AF_READ_ACCESS)
		return;

	Track *track = getTrack();

	track->f.bytesPerPacket = 0;
	track->f.framesPerPacket = kALACDefaultFramesPerPacket;

	const unsigned kALACSpecificConfigSize = 24;
	const unsigned kChannelAtomSize = 12;
	const unsigned kALACAudioChannelLayoutSize = 12;

	unsigned codecDataSize = kALACSpecificConfigSize;
	if (track->f.channelCount > 2)
		codecDataSize += kChannelAtomSize + kALACAudioChannelLayoutSize;
	m_codecData = new Buffer(codecDataSize);
	memset(m_codecData->data(), 0, m_codecData->size());

	AUpvlist pv = AUpvnew(2);

	AUpvsetparam(pv, 0, _AF_CODEC_DATA_SIZE);
	AUpvsetvaltype(pv, 0, AU_PVTYPE_LONG);
	long l = codecDataSize;
	AUpvsetval(pv, 0, &l);

	AUpvsetparam(pv, 1, _AF_CODEC_DATA);
	AUpvsetvaltype(pv, 1, AU_PVTYPE_PTR);
	void *v = m_codecData->data();
	AUpvsetval(pv, 1, &v);

	track->f.compressionParams = pv;

	track->m_packetTable = new PacketTable();
}
