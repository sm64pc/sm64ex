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
	This module implements Microsoft ADPCM compression.
*/

#include "config.h"
#include "MSADPCM.h"

#include <assert.h>
#include <cstdlib>
#include <limits>
#include <string.h>

#include "BlockCodec.h"
#include "Compiler.h"
#include "File.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "byteorder.h"
#include "util.h"
#include "../pcm.h"

struct ms_adpcm_state
{
	uint8_t predictorIndex;
	int delta;
	int16_t sample1, sample2;

	ms_adpcm_state()
	{
		predictorIndex = 0;
		delta = 16;
		sample1 = 0;
		sample2 = 0;
	}
};

class MSADPCM : public BlockCodec
{
public:
	static MSADPCM *createDecompress(Track *, File *, bool canSeek,
		bool headerless, AFframecount *chunkFrames);
	static MSADPCM *createCompress(Track *, File *, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual ~MSADPCM();

	bool initializeCoefficients();

	virtual const char *name() const OVERRIDE
	{
		return mode() == Compress ? "ms_adpcm_compress" : "ms_adpcm_decompress";
	}
	virtual void describe() OVERRIDE;

private:
	// m_coefficients is an array of m_numCoefficients ADPCM coefficient pairs.
	int m_numCoefficients;
	int16_t m_coefficients[256][2];

	ms_adpcm_state *m_state;

	MSADPCM(Mode mode, Track *track, File *fh, bool canSeek);

	int decodeBlock(const uint8_t *encoded, int16_t *decoded) OVERRIDE;
	int encodeBlock(const int16_t *decoded, uint8_t *encoded) OVERRIDE;
	void choosePredictorForBlock(const int16_t *decoded);
};

static inline int clamp(int x, int low, int high)
{
	if (x < low) return low;
	if (x > high) return high;
	return x;
}

static const int16_t adaptationTable[] =
{
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};

// Compute a linear PCM value from the given differential coded value.
static int16_t decodeSample(ms_adpcm_state &state,
	uint8_t code, const int16_t *coefficient)
{
	int linearSample = (state.sample1 * coefficient[0] +
		state.sample2 * coefficient[1]) >> 8;

	linearSample += ((code & 0x08) ? (code - 0x10) : code) * state.delta;

	linearSample = clamp(linearSample, MIN_INT16, MAX_INT16);

	int delta = (state.delta * adaptationTable[code]) >> 8;
	if (delta < 16)
		delta = 16;

	state.delta = delta;
	state.sample2 = state.sample1;
	state.sample1 = linearSample;

	return static_cast<int16_t>(linearSample);
}

// Compute a differential coded value from the given linear PCM sample.
static uint8_t encodeSample(ms_adpcm_state &state, int16_t sample,
	const int16_t *coefficient)
{
	int predictor = (state.sample1 * coefficient[0] +
		state.sample2 * coefficient[1]) >> 8;
	int code = sample - predictor;
	int bias = state.delta / 2;
	if (code < 0)
		bias = -bias;
	code = (code + bias) / state.delta;
	code = clamp(code, -8, 7) & 0xf;

	predictor += ((code & 0x8) ? (code - 0x10) : code) * state.delta;

	state.sample2 = state.sample1;
	state.sample1 = clamp(predictor, MIN_INT16, MAX_INT16);
	state.delta = (adaptationTable[code] * state.delta) >> 8;
	if (state.delta < 16)
		state.delta = 16;
	return code;
}

// Decode one block of MS ADPCM data.
int MSADPCM::decodeBlock(const uint8_t *encoded, int16_t *decoded)
{
	ms_adpcm_state decoderState[2];
	ms_adpcm_state *state[2];

	int channelCount = m_track->f.channelCount;

	// Calculate the number of bytes needed for decoded data.
	int outputLength = m_framesPerPacket * sizeof (int16_t) * channelCount;

	state[0] = &decoderState[0];
	if (channelCount == 2)
		state[1] = &decoderState[1];
	else
		state[1] = &decoderState[0];

	// Initialize block predictor.
	for (int i=0; i<channelCount; i++)
	{
		state[i]->predictorIndex = *encoded++;
		assert(state[i]->predictorIndex < m_numCoefficients);
	}

	// Initialize delta.
	for (int i=0; i<channelCount; i++)
	{
		state[i]->delta = (encoded[1]<<8) | encoded[0];
		encoded += sizeof (uint16_t);
	}

	// Initialize first two samples.
	for (int i=0; i<channelCount; i++)
	{
		state[i]->sample1 = (encoded[1]<<8) | encoded[0];
		encoded += sizeof (uint16_t);
	}

	for (int i=0; i<channelCount; i++)
	{
		state[i]->sample2 = (encoded[1]<<8) | encoded[0];
		encoded += sizeof (uint16_t);
	}

	const int16_t *coefficient[2] =
	{
		m_coefficients[state[0]->predictorIndex],
		m_coefficients[state[1]->predictorIndex]
	};

	for (int i=0; i<channelCount; i++)
		*decoded++ = state[i]->sample2;

	for (int i=0; i<channelCount; i++)
		*decoded++ = state[i]->sample1;

	/*
		The first two samples have already been 'decoded' in
		the block header.
	*/
	int samplesRemaining = (m_framesPerPacket - 2) * m_track->f.channelCount;

	while (samplesRemaining > 0)
	{
		uint8_t code;
		int16_t newSample;

		code = *encoded >> 4;
		newSample = decodeSample(*state[0], code, coefficient[0]);
		*decoded++ = newSample;

		code = *encoded & 0x0f;
		newSample = decodeSample(*state[1], code, coefficient[1]);
		*decoded++ = newSample;

		encoded++;
		samplesRemaining -= 2;
	}

	return outputLength;
}

int MSADPCM::encodeBlock(const int16_t *decoded, uint8_t *encoded)
{
	choosePredictorForBlock(decoded);

	int channelCount = m_track->f.channelCount;

	// Encode predictor.
	for (int c=0; c<channelCount; c++)
		*encoded++ = m_state[c].predictorIndex;

	// Encode delta.
	for (int c=0; c<channelCount; c++)
	{
		*encoded++ = m_state[c].delta & 0xff;
		*encoded++ = m_state[c].delta >> 8;
	}

	// Enccode first two samples.
	for (int c=0; c<channelCount; c++)
		m_state[c].sample2 = *decoded++;

	for (int c=0; c<channelCount; c++)
		m_state[c].sample1 = *decoded++;

	for (int c=0; c<channelCount; c++)
	{
		*encoded++ = m_state[c].sample1 & 0xff;
		*encoded++ = m_state[c].sample1 >> 8;
	}

	for (int c=0; c<channelCount; c++)
	{
		*encoded++ = m_state[c].sample2 & 0xff;
		*encoded++ = m_state[c].sample2 >> 8;
	}

	ms_adpcm_state *state[2] = { &m_state[0], &m_state[channelCount - 1] };
	const int16_t *coefficient[2] =
	{
		m_coefficients[state[0]->predictorIndex],
		m_coefficients[state[1]->predictorIndex]
	};

	int samplesRemaining = (m_framesPerPacket - 2) * m_track->f.channelCount;
	while (samplesRemaining > 0)
	{
		uint8_t code1 = encodeSample(*state[0], *decoded++, coefficient[0]);
		uint8_t code2 = encodeSample(*state[1], *decoded++, coefficient[1]);

		*encoded++ = (code1 << 4) | code2;
		samplesRemaining -= 2;
	}

	return m_bytesPerPacket;
}

void MSADPCM::choosePredictorForBlock(const int16_t *decoded)
{
	const int kPredictorSampleLength = 3;

	int channelCount = m_track->f.channelCount;

	for (int c=0; c<channelCount; c++)
	{
		int bestPredictorIndex = 0;
		int bestPredictorError = std::numeric_limits<int>::max();
		for (int k=0; k<m_numCoefficients; k++)
		{
			int a0 = m_coefficients[k][0];
			int a1 = m_coefficients[k][1];

			int currentPredictorError = 0;
			for (int i=2; i<2+kPredictorSampleLength; i++)
			{
				int error = std::abs(decoded[i*channelCount + c] -
					((a0 * decoded[(i-1)*channelCount + c] +
					a1 * decoded[(i-2)*channelCount + c]) >> 8));
				currentPredictorError += error;
			}

			currentPredictorError /= 4 * kPredictorSampleLength;

			if (currentPredictorError < bestPredictorError)
			{
				bestPredictorError = currentPredictorError;
				bestPredictorIndex = k;
			}

			if (!currentPredictorError)
				break;
		}

		if (bestPredictorError < 16)
			bestPredictorError = 16;

		m_state[c].predictorIndex = bestPredictorIndex;
		m_state[c].delta = bestPredictorError;
	}
}

void MSADPCM::describe()
{
	m_outChunk->f.byteOrder = _AF_BYTEORDER_NATIVE;
	m_outChunk->f.compressionType = AF_COMPRESSION_NONE;
	m_outChunk->f.compressionParams = AU_NULL_PVLIST;
}

MSADPCM::MSADPCM(Mode mode, Track *track, File *fh, bool canSeek) :
	BlockCodec(mode, track, fh, canSeek),
	m_numCoefficients(0),
	m_state(NULL)
{
	m_state = new ms_adpcm_state[m_track->f.channelCount];
}

MSADPCM::~MSADPCM()
{
	delete [] m_state;
}

bool MSADPCM::initializeCoefficients()
{
	AUpvlist pv = m_track->f.compressionParams;

	long l;
	if (_af_pv_getlong(pv, _AF_MS_ADPCM_NUM_COEFFICIENTS, &l))
	{
		m_numCoefficients = l;
	}
	else
	{
		_af_error(AF_BAD_CODEC_CONFIG, "number of coefficients not set");
		return false;
	}

	void *v;
	if (_af_pv_getptr(pv, _AF_MS_ADPCM_COEFFICIENTS, &v))
	{
		memcpy(m_coefficients, v, m_numCoefficients * 2 * sizeof (int16_t));
	}
	else
	{
		_af_error(AF_BAD_CODEC_CONFIG, "coefficient array not set");
		return false;
	}

	return true;
}

MSADPCM *MSADPCM::createDecompress(Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	assert(fh->tell() == track->fpos_first_frame);

	MSADPCM *msadpcm = new MSADPCM(Decompress, track, fh, canSeek);

	if (!msadpcm->initializeCoefficients())
	{
		delete msadpcm;
		return NULL;
	}

	*chunkFrames = msadpcm->m_framesPerPacket;

	return msadpcm;
}

MSADPCM *MSADPCM::createCompress(Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	assert(fh->tell() == track->fpos_first_frame);

	MSADPCM *msadpcm = new MSADPCM(Compress, track, fh, canSeek);

	if (!msadpcm->initializeCoefficients())
	{
		delete msadpcm;
		return NULL;
	}

	*chunkFrames = msadpcm->m_framesPerPacket;

	return msadpcm;
}

bool _af_ms_adpcm_format_ok (AudioFormat *f)
{
	if (f->channelCount != 1 && f->channelCount != 2)
	{
		_af_error(AF_BAD_COMPRESSION,
			"MS ADPCM compression requires 1 or 2 channels");
		return false;
	}

	if (f->sampleFormat != AF_SAMPFMT_TWOSCOMP || f->sampleWidth != 16)
	{
		_af_error(AF_BAD_COMPRESSION,
			"MS ADPCM compression requires 16-bit signed integer format");
		return false;
	}

	if (f->byteOrder != _AF_BYTEORDER_NATIVE)
	{
		_af_error(AF_BAD_COMPRESSION,
			"MS ADPCM compression requires native byte order");
		return false;
	}

	return true;
}

FileModule *_af_ms_adpcm_init_decompress (Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	return MSADPCM::createDecompress(track, fh, canSeek, headerless, chunkFrames);
}

FileModule *_af_ms_adpcm_init_compress (Track *track, File *fh,
	bool canSeek, bool headerless, AFframecount *chunkFrames)
{
	return MSADPCM::createCompress(track, fh, canSeek, headerless, chunkFrames);
}
