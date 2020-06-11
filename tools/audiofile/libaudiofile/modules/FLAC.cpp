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
#include "FLAC.h"

#include "Compiler.h"
#include "FileModule.h"
#include "Features.h"
#include "Track.h"
#include "byteorder.h"

#if ENABLE(FLAC)

#include <FLAC/stream_decoder.h>
#include <FLAC/stream_encoder.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <vector>

class FLACDecoder : public FileModule
{
public:
	static FLACDecoder *create(Track *track, File *file, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual ~FLACDecoder();

	virtual const char *name() const OVERRIDE { return "flac_decompress"; }

	virtual void describe() OVERRIDE;
	virtual void runPull() OVERRIDE;
	virtual void reset1() OVERRIDE;
	virtual void reset2() OVERRIDE;

	virtual bool handlesSeeking() const OVERRIDE { return true; }

private:
	FLACDecoder(Track *track, File *file, bool canSeek);

	FLAC__StreamDecoder *m_decoder;
	std::vector<int32_t *> m_buffer;
	int m_bufferedFrames, m_bufferedOffset;

	void convertAndInterleave(int offset, int frameCount);

	static FLAC__StreamDecoderReadStatus readCallback(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *clientData)
	{
		FLACDecoder *flac = static_cast<FLACDecoder *>(clientData);
		ssize_t result = flac->read(buffer, *bytes);
		if (result > 0)
		{
			*bytes = result;
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}

		*bytes = 0;
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}

	static FLAC__StreamDecoderSeekStatus seekCallback(const FLAC__StreamDecoder *, FLAC__uint64 absoluteByteOffset, void *clientData)
	{
		FLACDecoder *flac = static_cast<FLACDecoder *>(clientData);
		if (flac->seek(absoluteByteOffset) < 0)
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	}

	static FLAC__StreamDecoderTellStatus tellCallback(const FLAC__StreamDecoder *, FLAC__uint64 *absoluteByteOffset, void *clientData)
	{
		FLACDecoder *flac = static_cast<FLACDecoder *>(clientData);
		off_t result = flac->tell();
		if (result < 0)
			return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
		*absoluteByteOffset = static_cast<FLAC__uint64>(result);
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}

	static FLAC__StreamDecoderLengthStatus lengthCallback(const FLAC__StreamDecoder *, FLAC__uint64 *length, void *clientData)
	{
		FLACDecoder *flac = static_cast<FLACDecoder *>(clientData);
		off_t result = flac->length();
		if (result < 0)
			return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
		*length = result;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}

	static FLAC__bool eofCallback(const FLAC__StreamDecoder *, void *clientData)
	{
		FLACDecoder *flac = static_cast<FLACDecoder *>(clientData);
		return flac->tell() == flac->length();
	}

	static FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData)
	{
		FLACDecoder *flac = static_cast<FLACDecoder *>(clientData);
		flac->didDecodeFrame(frame, buffer);
		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}

	static void metadataCallback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *clientData)
	{
	}

	static void errorCallback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *clientData)
	{
		_af_error(AF_BAD_CODEC_CONFIG, "FLAC decoding error %d", status);
	}

	void didDecodeFrame(const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
	{
		m_bufferedFrames = frame->header.blocksize;
		m_bufferedOffset = 0;
		for (unsigned c=0; c<frame->header.channels; c++)
			memcpy(m_buffer[c], buffer[c], frame->header.blocksize * sizeof (int32_t));

		m_track->nextfframe += frame->header.blocksize;
	}
};

FLACDecoder *FLACDecoder::create(Track *track, File *file, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	return new FLACDecoder(track, file, canSeek);
}

FLACDecoder::FLACDecoder(Track *track, File *file, bool canSeek) :
	FileModule(Decompress, track, file, canSeek),
	m_decoder(NULL),
	m_bufferedFrames(0),
	m_bufferedOffset(0)
{
	m_decoder = FLAC__stream_decoder_new();

	if (FLAC__stream_decoder_init_stream(m_decoder,
		readCallback,
		seekCallback,
		tellCallback,
		lengthCallback,
		eofCallback,
		writeCallback,
		metadataCallback,
		errorCallback,
		this) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not initialize FLAC decoder");
		return;
	}

	m_buffer.resize(m_track->f.channelCount);
	for (int c=0; c<m_track->f.channelCount; c++)
		m_buffer[c] = new int32_t[FLAC__MAX_BLOCK_SIZE];
}

FLACDecoder::~FLACDecoder()
{
	if (m_decoder)
	{
		FLAC__stream_decoder_delete(m_decoder);
		m_decoder = NULL;
	}

	for (size_t i=0; i<m_buffer.size(); i++)
		delete [] m_buffer[i];
}

void FLACDecoder::describe()
{
	m_outChunk->f.byteOrder = _AF_BYTEORDER_NATIVE;
	m_outChunk->f.compressionType = AF_COMPRESSION_NONE;
	m_outChunk->f.compressionParams = AU_NULL_PVLIST;
}

void FLACDecoder::convertAndInterleave(int offset, int frameCount)
{
	int channelCount = m_outChunk->f.channelCount;

	if (m_track->f.sampleWidth == 16)
	{
		int16_t *out = static_cast<int16_t *>(m_outChunk->buffer);
		for (int i=0; i<frameCount; i++)
		{
			for (int c=0; c<channelCount; c++)
			{
				int outIndex = (offset+i) * channelCount + c;
				out[outIndex] = m_buffer[c][m_bufferedOffset + i];
			}
		}
	}
	else if (m_track->f.sampleWidth == 24)
	{
		uint8_t *out = static_cast<uint8_t *>(m_outChunk->buffer);
		for (int i=0; i<frameCount; i++)
		{
			for (int c=0; c<channelCount; c++)
			{
				int32_t in = m_buffer[c][m_bufferedOffset + i];
				uint8_t c0, c1, c2;

#ifdef WORDS_BIGENDIAN
				c0 = (in >> 16) & 0xff;
				c1 = (in >> 8) & 0xff;
				c2 = in & 0xff;
#else
				c2 = (in >> 16) & 0xff;
				c1 = (in >> 8) & 0xff;
				c0 = in & 0xff;
#endif

				int outIndex = (offset+i) * channelCount + c;
				out[3*outIndex] = c0;
				out[3*outIndex+1] = c1;
				out[3*outIndex+2] = c2;
			}
		}
	}

	m_bufferedOffset += frameCount;
}

void FLACDecoder::runPull()
{
	int framesToRead = m_outChunk->frameCount;
	int offset = 0;
	while (framesToRead > 0)
	{
		int bufferedFramesToRead = std::min<int>(framesToRead,
			m_bufferedFrames - m_bufferedOffset);
		convertAndInterleave(offset, bufferedFramesToRead);
		offset += bufferedFramesToRead;
		framesToRead -= bufferedFramesToRead;

		if (framesToRead > 0)
		{
			if (!FLAC__stream_decoder_process_single(m_decoder))
				break;
			if (FLAC__stream_decoder_get_state(m_decoder) >= FLAC__STREAM_DECODER_END_OF_STREAM)
				break;
		}
	}
}

void FLACDecoder::reset1()
{
}

void FLACDecoder::reset2()
{
	if (!FLAC__stream_decoder_seek_absolute(m_decoder, m_track->nextfframe))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not seek to frame %jd",
			static_cast<intmax_t>(m_track->nextfframe));
	}
}

class FLACEncoder : public FileModule
{
public:
	static FLACEncoder *create(Track *track, File *file, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual ~FLACEncoder();

	virtual const char *name() const OVERRIDE { return "flac_compress"; }

	virtual void describe() OVERRIDE;
	virtual void runPush() OVERRIDE;
	virtual void sync1() OVERRIDE;
	virtual void sync2() OVERRIDE;

	virtual bool handlesSeeking() const OVERRIDE { return true; }

private:
	FLAC__StreamEncoder *m_encoder;
	FLAC__int32 *m_buffer;

	FLACEncoder(Track *track, File *file, bool canSeek);

	void convert16To32();
	void convert24To32();

	static FLAC__StreamEncoderSeekStatus seekCallback(const FLAC__StreamEncoder *, FLAC__uint64 absoluteByteOffset, void *clientData)
	{
		FLACEncoder *flac = static_cast<FLACEncoder *>(clientData);
		off_t result = flac->seek(absoluteByteOffset);
		if (result < 0)
			return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
		return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
	}

	static FLAC__StreamEncoderTellStatus tellCallback(const FLAC__StreamEncoder *, FLAC__uint64 *absoluteByteOffset, void *clientData)
	{
		FLACEncoder *flac = static_cast<FLACEncoder *>(clientData);
		off_t offset = flac->tell();
		if (offset < 0)
			return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
		*absoluteByteOffset = static_cast<FLAC__uint64>(offset);
		return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
	}

	static FLAC__StreamEncoderWriteStatus writeCallback(const FLAC__StreamEncoder *, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned currentFrame, void *clientData)
	{
		FLACEncoder *flac = static_cast<FLACEncoder *>(clientData);
		ssize_t result = flac->write(buffer, bytes);
		if (result == static_cast<ssize_t>(bytes))
			return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
		return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
	}
};

FLACEncoder::FLACEncoder(Track *track, File *file, bool canSeek) :
	FileModule(Compress, track, file, canSeek),
	m_encoder(NULL),
	m_buffer(NULL)
{
	m_encoder = FLAC__stream_encoder_new();
	if (!m_encoder)
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not create encoder");
		return;
	}

	if (!FLAC__stream_encoder_set_channels(m_encoder, m_track->f.channelCount))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not set channel count");
		return;
	}

	if (!FLAC__stream_encoder_set_sample_rate(m_encoder, m_track->f.sampleRate))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not set sample rate");
		return;
	}

	if (!FLAC__stream_encoder_set_bits_per_sample(m_encoder, m_track->f.sampleWidth))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not set sample width");
		return;
	}

	if (FLAC__stream_encoder_init_stream(m_encoder,
		writeCallback,
		seekCallback,
		tellCallback,
		NULL,
		this) != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not initialize FLAC encoder");
		return;
	}

	m_buffer = new int32_t[FLAC__MAX_BLOCK_SIZE * m_track->f.channelCount];
}

FLACEncoder::~FLACEncoder()
{
	if (m_encoder)
	{
		FLAC__stream_encoder_delete(m_encoder);
		m_encoder = NULL;
	}

	delete [] m_buffer;
}

FLACEncoder *FLACEncoder::create(Track *track, File *file, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	return new FLACEncoder(track, file, canSeek);
}

void FLACEncoder::describe()
{
	m_outChunk->f.byteOrder = _AF_BYTEORDER_NATIVE;
	m_outChunk->f.compressionType = AF_COMPRESSION_FLAC;
}

void FLACEncoder::runPush()
{
	if (m_track->f.sampleWidth == 16)
		convert16To32();
	else if (m_track->f.sampleWidth == 24)
		convert24To32();

	if (!FLAC__stream_encoder_process_interleaved(m_encoder, m_buffer,
		m_inChunk->frameCount))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not encode data into FLAC stream");
	}

	m_track->nextfframe += m_inChunk->frameCount;
	m_track->totalfframes = m_track->nextfframe;
}

void FLACEncoder::sync1()
{
}

void FLACEncoder::sync2()
{
	if (!FLAC__stream_encoder_finish(m_encoder))
	{
		_af_error(AF_BAD_CODEC_CONFIG, "could not finish encoding");
	}
}

void FLACEncoder::convert16To32()
{
	int channelCount = m_track->f.channelCount;
	const int16_t *src = static_cast<const int16_t *>(m_inChunk->buffer);
	for (unsigned i=0; i<m_inChunk->frameCount; i++)
		for (int c=0; c<channelCount; c++)
			m_buffer[channelCount * i + c] = src[channelCount * i + c];
}

void FLACEncoder::convert24To32()
{
	int channelCount = m_track->f.channelCount;
	const uint8_t *src = static_cast<const uint8_t *>(m_inChunk->buffer);
	for (unsigned i=0; i<m_inChunk->frameCount; i++)
		for (int c=0; c<channelCount; c++)
		{
			int srcIndex = channelCount * i + c; 
			int32_t t =
#ifdef WORDS_BIGENDIAN
				(src[3*srcIndex] << 24) |
				(src[3*srcIndex+1] << 16) |
				src[3*srcIndex+2] << 8;
#else
				(src[3*srcIndex+2] << 24) |
				(src[3*srcIndex+1] << 16) |
				src[3*srcIndex] << 8;
#endif
			m_buffer[channelCount * i + c] = t >> 8;
		}
}

bool _af_flac_format_ok(AudioFormat *f)
{
	if (f->channelCount > static_cast<int>(FLAC__MAX_CHANNELS))
	{
		_af_error(AF_BAD_COMPRESSION,
			"FLAC compression supports a maximum of 8 channels");
		return false;
	}

	if (f->sampleFormat != AF_SAMPFMT_TWOSCOMP ||
		(f->sampleWidth != 16 && f->sampleWidth != 24))
	{
		_af_error(AF_BAD_COMPRESSION,
			"FLAC compression requires 16- or 24-bit signed integer format");
		return false;
	}

	if (f->byteOrder != _AF_BYTEORDER_NATIVE)
	{
		_af_error(AF_BAD_COMPRESSION,
			"FLAC compression requires native byte order");
		return false;
	}

	return true;
}

#else

bool _af_flac_format_ok(AudioFormat *)
{
	return false;
}

#endif

FileModule *_af_flac_init_compress(Track *track, File *file, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
#if ENABLE(FLAC)
	return FLACEncoder::create(track, file, canSeek, headerless, chunkFrames);
#else
	return NULL;
#endif
}

FileModule *_af_flac_init_decompress(Track *track, File *file, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
#if ENABLE(FLAC)
	return FLACDecoder::create(track, file, canSeek, headerless, chunkFrames);
#else
	return NULL;
#endif
}
