/*
	Audio File Library
	Copyright (C) 2010-2013, Michael Pruett <michael@68k.org>
	Copyright (C) 2001, Silicon Graphics, Inc.

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
	This module implements IMA ADPCM compression.
*/

#include "config.h"
#include "IMA.h"

#include <assert.h>

#include <audiofile.h>

#include "BlockCodec.h"
#include "Compiler.h"
#include "File.h"
#include "Track.h"
#include "afinternal.h"
#include "byteorder.h"
#include "util.h"
#include "../pcm.h"

struct adpcmState
{
	int previousValue;	// previous output value
	int index;			// index into step table

	adpcmState()
	{
		previousValue = 0;
		index = 0;
	}
};

class IMA : public BlockCodec
{
public:
	static IMA *createDecompress(Track *track, File *fh, bool canSeek,
		bool headerless, AFframecount *chunkFrames);
	static IMA *createCompress(Track *track, File *fh, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual ~IMA();

	virtual const char *name() const OVERRIDE
	{
		return mode() == Compress ?
			"ima_adpcm_compress" : "ima_adpcm_decompress";
	}
	virtual void describe() OVERRIDE;

private:
	int m_imaType;
	adpcmState *m_adpcmState;

	IMA(Mode, Track *, File *fh, bool canSeek);

	int decodeBlock(const uint8_t *encoded, int16_t *decoded) OVERRIDE;
	int decodeBlockWAVE(const uint8_t *encoded, int16_t *decoded);
	int decodeBlockQT(const uint8_t *encoded, int16_t *decoded);

	int encodeBlock(const int16_t *input, uint8_t *output) OVERRIDE;
	int encodeBlockWAVE(const int16_t *input, uint8_t *output);
	int encodeBlockQT(const int16_t *input, uint8_t *output);
};

IMA::IMA(Mode mode, Track *track, File *fh, bool canSeek) :
	BlockCodec(mode, track, fh, canSeek),
	m_imaType(0)
{
	AUpvlist pv = (AUpvlist) track->f.compressionParams;

	m_framesPerPacket = track->f.framesPerPacket;
	m_bytesPerPacket = track->f.bytesPerPacket;

	long l;
	if (_af_pv_getlong(pv, _AF_IMA_ADPCM_TYPE, &l))
		m_imaType = l;

	m_adpcmState = new adpcmState[track->f.channelCount];
}

IMA::~IMA()
{
	delete [] m_adpcmState;
}

int IMA::decodeBlock(const uint8_t *encoded, int16_t *decoded)
{
	if (m_imaType == _AF_IMA_ADPCM_TYPE_WAVE)
		return decodeBlockWAVE(encoded, decoded);
	else if (m_imaType == _AF_IMA_ADPCM_TYPE_QT)
		return decodeBlockQT(encoded, decoded);
	return 0;
}

static const int8_t indexTable[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

static const int16_t stepTable[89] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static inline int clamp(int x, int low, int high)
{
	if (x < low) return low;
	if (x > high) return high;
	return x;
}

static inline int16_t decodeSample(adpcmState &state, uint8_t code)
{
	int step = stepTable[state.index];

	int diff = step >> 3;
	if (code & 4) diff += step;
	if (code & 2) diff += step>>1;
	if (code & 1) diff += step>>2;

	int predictor = state.previousValue;
	if (code & 8)
		predictor -= diff;
	else
		predictor += diff;

	state.previousValue = clamp(predictor, MIN_INT16, MAX_INT16);
	state.index = clamp(state.index + indexTable[code], 0, 88);
	return state.previousValue;
}

int IMA::decodeBlockWAVE(const uint8_t *encoded, int16_t *decoded)
{
	int channelCount = m_track->f.channelCount;

	for (int c=0; c<channelCount; c++)
	{
		m_adpcmState[c].previousValue = (encoded[1]<<8) | encoded[0];
		if (encoded[1] & 0x80)
			m_adpcmState[c].previousValue -= 0x10000;

		m_adpcmState[c].index = encoded[2];

		*decoded++ = m_adpcmState[c].previousValue;

		encoded += 4;
	}

	for (int n=0; n<m_framesPerPacket - 1; n += 8)
	{
		for (int c=0; c<channelCount; c++)
		{
			int16_t *output = decoded + c;
			for (int s=0; s<4; s++)
			{
				*output = decodeSample(m_adpcmState[c], *encoded & 0xf);
				output += channelCount;
				*output = decodeSample(m_adpcmState[c], *encoded >> 4);
				output += channelCount;
				encoded++;
			}
		}

		decoded += channelCount * 8;
	}

	return m_framesPerPacket * channelCount * sizeof (int16_t);
}


int IMA::decodeBlockQT(const uint8_t *encoded, int16_t *decoded)
{
	int channelCount = m_track->f.channelCount;

	for (int c=0; c<channelCount; c++)
	{
		adpcmState state;
		int predictor = (encoded[0] << 8) | (encoded[1] & 0x80);
		if (predictor & 0x8000)
			predictor -= 0x10000;

		state.previousValue = clamp(predictor, MIN_INT16, MAX_INT16);
		state.index = encoded[1] & 0x7f;
		encoded += 2;

		for (int n=0; n<m_framesPerPacket; n+=2)
		{
			uint8_t e = *encoded;
			decoded[n*channelCount + c] = decodeSample(state, e & 0xf);
			decoded[(n+1)*channelCount + c] = decodeSample(state, e >> 4);
			encoded++;
		}
	}

	return m_framesPerPacket * channelCount * sizeof (int16_t);
}

int IMA::encodeBlock(const int16_t *input, uint8_t *output)
{
	if (m_imaType == _AF_IMA_ADPCM_TYPE_WAVE)
		return encodeBlockWAVE(input, output);
	else if (m_imaType == _AF_IMA_ADPCM_TYPE_QT)
		return encodeBlockQT(input, output);
	return 0;
}

static inline uint8_t encodeSample(adpcmState &state, int16_t sample)
{
	int step = stepTable[state.index];
	int diff = sample - state.previousValue;
	int vpdiff = step >> 3;
	uint8_t code = 0;
	if (diff < 0)
	{
		code = 8;
		diff = -diff;
	}
	if (diff >= step)
	{
		code |= 4;
		diff -= step;
		vpdiff += step;
	}
	step >>= 1;
	if (diff >= step)
	{
		code |= 2;
		diff -= step;
		vpdiff += step;
	}
	step >>= 1;
	if (diff >= step)
	{
		code |= 1;
		vpdiff += step;
	}

	if (code & 8)
		vpdiff = -vpdiff;
	state.previousValue = clamp(state.previousValue + vpdiff,
		MIN_INT16, MAX_INT16);

	state.index = clamp(state.index + indexTable[code], 0, 88);
	return code & 0xf;
}

int IMA::encodeBlockWAVE(const int16_t *input, uint8_t *output)
{
	int channelCount = m_track->f.channelCount;

	for (int c=0; c<channelCount; c++)
	{
		output[0] = m_adpcmState[c].previousValue & 0xff;
		output[1] = m_adpcmState[c].previousValue >> 8;
		output[2] = m_adpcmState[c].index;
		output[3] = 0;

		output += 4;
	}

	for (int n=0; n<m_framesPerPacket - 1; n += 8)
	{
		for (int c=0; c<channelCount; c++)
		{
			const int16_t *currentInput = input + c;
			for (int s=0; s<4; s++)
			{
				uint8_t encodedValue = encodeSample(m_adpcmState[c], *currentInput);
				currentInput += channelCount;
				encodedValue |= encodeSample(m_adpcmState[c], *currentInput) << 4;
				currentInput += channelCount;
				*output++ = encodedValue;
			}
		}

		input += channelCount * 8;
	}

	return m_bytesPerPacket;
}

int IMA::encodeBlockQT(const int16_t *input, uint8_t *output)
{
	int channelCount = m_track->f.channelCount;

	for (int c=0; c<channelCount; c++)
	{
		adpcmState state = m_adpcmState[c];

		state.previousValue &= ~0x7f;

		output[0] = (state.previousValue >> 8) & 0xff;
		output[1] = (state.previousValue & 0x80) | (state.index & 0x7f);
		output += 2;

		for (int n=0; n<m_framesPerPacket; n+=2)
		{
			uint8_t encoded = encodeSample(state, input[n*channelCount + c]);
			encoded |= encodeSample(state, input[(n+1)*channelCount + c]) << 4;
			*output++ = encoded;
		}

		m_adpcmState[c] = state;
	}

	return m_bytesPerPacket;
}

bool _af_ima_adpcm_format_ok (AudioFormat *f)
{
	if (f->channelCount != 1 && f->channelCount != 2)
	{
		_af_error(AF_BAD_COMPRESSION,
			"IMA ADPCM compression requires 1 or 2 channels");
		return false;
	}

	if (f->sampleFormat != AF_SAMPFMT_TWOSCOMP || f->sampleWidth != 16)
	{
		_af_error(AF_BAD_COMPRESSION,
			"IMA ADPCM compression requires 16-bit signed integer format");
		return false;
	}

	if (f->byteOrder != _AF_BYTEORDER_NATIVE)
	{
		_af_error(AF_BAD_COMPRESSION,
			"IMA ADPCM compression requires native byte order");
		return false;
	}

	return true;
}

void IMA::describe()
{
	m_outChunk->f.byteOrder = _AF_BYTEORDER_NATIVE;
	m_outChunk->f.compressionType = AF_COMPRESSION_NONE;
	m_outChunk->f.compressionParams = AU_NULL_PVLIST;
}

IMA *IMA::createDecompress(Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	assert(fh->tell() == track->fpos_first_frame);

	IMA *ima = new IMA(Decompress, track, fh, canSeek);

	if (!ima->m_imaType)
	{
		_af_error(AF_BAD_CODEC_CONFIG, "IMA type not set");
		delete ima;
		return NULL;
	}

	*chunkFrames = ima->m_framesPerPacket;
	return ima;
}

IMA *IMA::createCompress(Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	assert(fh->tell() == track->fpos_first_frame);

	IMA *ima = new IMA(Compress, track, fh, canSeek);

	if (!ima->m_imaType)
	{
		_af_error(AF_BAD_CODEC_CONFIG, "IMA type not set");
		delete ima;
		return NULL;
	}

	*chunkFrames = ima->m_framesPerPacket;
	return ima;
}

FileModule *_af_ima_adpcm_init_decompress(Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	return IMA::createDecompress(track, fh, canSeek, headerless, chunkFrames);
}

FileModule *_af_ima_adpcm_init_compress(Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	return IMA::createCompress(track, fh, canSeek, headerless, chunkFrames);
}
