/*
	Audio File Library
	Copyright (C) 2013 Michael Pruett <michael@68k.org>

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
#include "ALAC.h"

#include "Buffer.h"
#include "Compiler.h"
#include "File.h"
#include "FileModule.h"
#include "PacketTable.h"
#include "SimpleModule.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "byteorder.h"
#include "compression.h"
#include "units.h"
#include "util.h"

#include "../alac/ALACBitUtilities.h"
#include "../alac/ALACDecoder.h"
#include "../alac/ALACEncoder.h"

#include <assert.h>
#include <string.h>

enum
{
	kALACFormatFlag_16BitSourceData = 1,
	kALACFormatFlag_20BitSourceData = 2,
	kALACFormatFlag_24BitSourceData = 3,
	kALACFormatFlag_32BitSourceData = 4
};

class ALAC : public FileModule
{
public:
	static ALAC *createDecompress(Track *, File *, bool canSeek,
		bool headerless, AFframecount *chunkFrames);
	static ALAC *createCompress(Track *, File *, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual ~ALAC();

	virtual const char *name() const OVERRIDE
	{
		return mode() == Compress ?  "alac_compress" : "alac_decompress";
	}
	virtual void describe() OVERRIDE;
	virtual void runPull() OVERRIDE;
	virtual void reset1() OVERRIDE;
	virtual void reset2() OVERRIDE;
	virtual void runPush() OVERRIDE;
	virtual void sync1() OVERRIDE;
	virtual void sync2() OVERRIDE;
	virtual int bufferSize() const OVERRIDE;

private:
	AFframecount m_framesToIgnore;
	AFfileoffset m_savedPositionNextFrame;
	AFframecount m_savedNextFrame;

	SharedPtr<Buffer> m_codecData;
	ALACDecoder *m_decoder;
	ALACEncoder *m_encoder;
	int m_currentPacket;

	ALAC(Mode mode, Track *track, File *fh, bool canSeek, Buffer *codecData);
	void initDecoder();
	void initEncoder();

	AudioFormatDescription outputFormat() const;
};

ALAC::ALAC(Mode mode, Track *track, File *fh, bool canSeek, Buffer *codecData) :
	FileModule(mode, track, fh, canSeek),
	m_savedPositionNextFrame(-1),
	m_savedNextFrame(-1),
	m_codecData(codecData),
	m_decoder(NULL),
	m_encoder(NULL),
	m_currentPacket(0)
{
	if (mode == Decompress)
		initDecoder();
	else
		initEncoder();
}

ALAC::~ALAC()
{
	delete m_decoder;
	delete m_encoder;
}

void ALAC::initDecoder()
{
	m_decoder = new ALACDecoder();
	m_decoder->Init(m_codecData->data(), m_codecData->size());
}

void ALAC::initEncoder()
{
	m_encoder = new ALACEncoder();
	m_encoder->SetFrameSize(m_track->f.framesPerPacket);
	m_encoder->InitializeEncoder(outputFormat());

	uint32_t cookieSize = m_encoder->GetMagicCookieSize(m_track->f.channelCount);
	assert(cookieSize == m_codecData->size());
	m_encoder->GetMagicCookie(m_codecData->data(), &cookieSize);

	void *v = NULL;
	_af_pv_getptr(m_track->f.compressionParams, _AF_CODEC_DATA, &v);
	::memcpy(v, m_codecData->data(), cookieSize);
}

AudioFormatDescription ALAC::outputFormat() const
{
	AudioFormatDescription outputFormat;
	outputFormat.mSampleRate = m_track->f.sampleRate;
	outputFormat.mFormatID = kALACFormatAppleLossless;
	switch (m_track->f.sampleWidth)
	{
		case 16:
			outputFormat.mFormatFlags = kALACFormatFlag_16BitSourceData; break;
		case 20:
			outputFormat.mFormatFlags = kALACFormatFlag_20BitSourceData; break;
		case 24:
			outputFormat.mFormatFlags = kALACFormatFlag_24BitSourceData; break;
		case 32:
			outputFormat.mFormatFlags = kALACFormatFlag_32BitSourceData; break;
		default:
			outputFormat.mFormatFlags = 0; break;
	}
	outputFormat.mFramesPerPacket = m_track->f.framesPerPacket;
	outputFormat.mChannelsPerFrame = m_track->f.channelCount;
	outputFormat.mBytesPerPacket = 0;
	outputFormat.mBytesPerFrame = 0;
	outputFormat.mBitsPerChannel = 0;
	outputFormat.mReserved = 0;
	return outputFormat;
}

void ALAC::describe()
{
	m_outChunk->f.byteOrder = _AF_BYTEORDER_NATIVE;
	m_outChunk->f.compressionType = AF_COMPRESSION_NONE;
	m_outChunk->f.compressionParams = AU_NULL_PVLIST;
}

ALAC *ALAC::createDecompress(Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	assert(fh->tell() == track->fpos_first_frame);

	AUpvlist pv = (AUpvlist) track->f.compressionParams;
	long codecDataSize;
	if (!_af_pv_getlong(pv, _AF_CODEC_DATA_SIZE, &codecDataSize))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "codec data size not set");
		return NULL;
	}

	SharedPtr<Buffer> codecData = new Buffer(codecDataSize);

	void *data;
	if (!_af_pv_getptr(pv, _AF_CODEC_DATA, &data))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "codec data not set");
		return NULL;
	}

	memcpy(codecData->data(), data, codecDataSize);

	*chunkFrames = track->f.framesPerPacket;

	return new ALAC(Decompress, track, fh, canSeek, codecData.get());
}

ALAC *ALAC::createCompress(Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	assert(fh->tell() == track->fpos_first_frame);

	AUpvlist pv = (AUpvlist) track->f.compressionParams;
	long codecDataSize;
	if (!_af_pv_getlong(pv, _AF_CODEC_DATA_SIZE, &codecDataSize))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "codec data size not set");
		return NULL;
	}

	SharedPtr<Buffer> codecData = new Buffer(codecDataSize);

	void *data;
	if (!_af_pv_getptr(pv, _AF_CODEC_DATA, &data))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "codec data not set");
		return NULL;
	}

	memcpy(codecData->data(), data, codecDataSize);

	*chunkFrames = track->f.framesPerPacket;

	return new ALAC(Compress, track, fh, canSeek, codecData.get());
}

void ALAC::runPull()
{
	SharedPtr<PacketTable> packetTable = m_track->m_packetTable;
	if (m_currentPacket >= static_cast<int>(packetTable->numPackets()))
	{
		m_outChunk->frameCount = 0;
		return;
	}
	assert(m_currentPacket < static_cast<int>(packetTable->numPackets()));

	ssize_t bytesPerPacket = packetTable->bytesPerPacket(m_currentPacket);
	assert(bytesPerPacket <= bufferSize());

	if (read(m_inChunk->buffer, bytesPerPacket) < bytesPerPacket)
	{
		reportReadError(0, m_track->f.framesPerPacket);
		return;
	}

	BitBuffer bitBuffer;
	BitBufferInit(&bitBuffer, static_cast<uint8_t *>(m_inChunk->buffer),
		bytesPerPacket);

	uint32_t numFrames;
	m_decoder->Decode(&bitBuffer, static_cast<uint8_t *>(m_outChunk->buffer),
		m_track->f.framesPerPacket, m_track->f.channelCount, &numFrames);
	m_outChunk->frameCount = numFrames;

	m_currentPacket++;
}

void ALAC::reset1()
{
	AFframecount nextFrame = m_track->nextfframe;
	m_currentPacket = nextFrame / m_track->f.framesPerPacket;
	m_track->nextfframe = m_currentPacket * m_track->f.framesPerPacket;
	m_framesToIgnore = nextFrame - m_track->nextfframe;
}

void ALAC::reset2()
{
	m_track->fpos_next_frame = m_track->fpos_first_frame +
		m_track->m_packetTable->startOfPacket(m_currentPacket);
	m_track->frames2ignore += m_framesToIgnore;
}

int ALAC::bufferSize() const
{
	return m_track->f.framesPerPacket * m_track->f.channelCount *
		((10 + m_track->f.sampleWidth) / 8) + 1;
}

void ALAC::runPush()
{
	AudioFormatDescription inputFormat;
	inputFormat.mSampleRate = m_track->f.sampleRate;
	inputFormat.mFormatID = kALACFormatLinearPCM;
	inputFormat.mFormatFlags = kALACFormatFlagsNativeEndian;
	inputFormat.mBytesPerPacket = _af_format_frame_size_uncompressed(&m_track->f, false);
	inputFormat.mFramesPerPacket = 1;
	inputFormat.mBytesPerFrame = _af_format_frame_size_uncompressed(&m_track->f, false);
	inputFormat.mChannelsPerFrame = m_track->f.channelCount;
	inputFormat.mBitsPerChannel = m_track->f.sampleWidth;
	inputFormat.mReserved = 0;

	int32_t numBytes = m_inChunk->frameCount * inputFormat.mBytesPerFrame;
	int32_t result = m_encoder->Encode(inputFormat, outputFormat(),
		static_cast<uint8_t *>(m_inChunk->buffer),
		static_cast<uint8_t *>(m_outChunk->buffer),
		&numBytes);
	if (result)
	{
		_af_error(AF_BAD_CODEC_STATE, "error encoding ALAC audio data");
		m_track->filemodhappy = false;
		return;
	}

	assert(numBytes <= bufferSize());

	ssize_t bytesWritten = write(m_outChunk->buffer, numBytes);
	if (bytesWritten != numBytes)
	{
		reportWriteError(0, m_track->f.framesPerPacket);
		return;
	}

	PacketTable *packetTable = m_track->m_packetTable.get();

	packetTable->append(numBytes);

	packetTable->setNumValidFrames(packetTable->numValidFrames() +
		m_inChunk->frameCount);
}

void ALAC::sync1()
{
	m_savedPositionNextFrame = m_track->fpos_next_frame;
	m_savedNextFrame = m_track->nextfframe;
}

void ALAC::sync2()
{
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));

	m_track->fpos_after_data = tell();

	m_track->fpos_next_frame = m_savedPositionNextFrame;
	m_track->nextfframe = m_savedNextFrame;
}

bool _af_alac_format_ok (AudioFormat *f)
{
	if (f->channelCount > kALACMaxChannels)
	{
		_af_error(AF_BAD_CHANNELS,
			"ALAC compression supports a maximum of 8 channels");
		return false;
	}

	if (f->sampleFormat != AF_SAMPFMT_TWOSCOMP)
	{
		_af_error(AF_BAD_COMPRESSION,
			"ALAC compression requires signed integer audio data");
		return false;
	}

	if (f->sampleWidth != 16 &&
		f->sampleWidth != 20 &&
		f->sampleWidth != 24 &&
		f->sampleWidth != 32)
	{
		_af_error(AF_BAD_WIDTH,
			"ALAC compression requires 16, 20, 24, or 32 bits per sample");
		return false;
	}

	if (f->byteOrder != _AF_BYTEORDER_NATIVE)
	{
		_af_error(AF_BAD_COMPRESSION,
			"ALAC compression requires native-endian format");
		f->byteOrder = _AF_BYTEORDER_NATIVE;
	}

	return true;
}

FileModule *_af_alac_init_decompress (Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	return ALAC::createDecompress(track, fh, canSeek, headerless, chunkFrames);
}

FileModule *_af_alac_init_compress (Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	return ALAC::createCompress(track, fh, canSeek, headerless, chunkFrames);
}
