/*
	Audio File Library
	Copyright (C) 1998-2000, 2003-2004, 2010-2013, Michael Pruett <michael@68k.org>
	Copyright (C) 2000-2002, Silicon Graphics, Inc.
	Copyright (C) 2002-2003, Davy Durham

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
	WAVE.cpp

	This file contains code for reading and writing RIFF WAVE format
	sound files.
*/

#include "config.h"
#include "WAVE.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "File.h"
#include "Instrument.h"
#include "Marker.h"
#include "Setup.h"
#include "Tag.h"
#include "Track.h"
#include "UUID.h"
#include "byteorder.h"
#include "util.h"

/* These constants are from RFC 2361. */
enum
{
	WAVE_FORMAT_UNKNOWN = 0x0000,	/* Microsoft Unknown Wave Format */
	WAVE_FORMAT_PCM = 0x0001,	/* Microsoft PCM Format */
	WAVE_FORMAT_ADPCM = 0x0002,	/* Microsoft ADPCM Format */
	WAVE_FORMAT_IEEE_FLOAT = 0x0003,	/* IEEE Float */
	WAVE_FORMAT_VSELP = 0x0004,	/* Compaq Computer's VSELP */
	WAVE_FORMAT_IBM_CVSD = 0x0005,	/* IBM CVSD */
	WAVE_FORMAT_ALAW = 0x0006,	/* Microsoft ALAW */
	WAVE_FORMAT_MULAW = 0x0007,	/* Microsoft MULAW */
	WAVE_FORMAT_OKI_ADPCM = 0x0010,	/* OKI ADPCM */
	WAVE_FORMAT_DVI_ADPCM = 0x0011,	/* Intel's DVI ADPCM */
	WAVE_FORMAT_MEDIASPACE_ADPCM = 0x0012,	/* Videologic's MediaSpace ADPCM */
	WAVE_FORMAT_SIERRA_ADPCM = 0x0013,	/* Sierra ADPCM */
	WAVE_FORMAT_G723_ADPCM = 0x0014,	/* G.723 ADPCM */
	WAVE_FORMAT_DIGISTD = 0x0015,	/* DSP Solutions' DIGISTD */
	WAVE_FORMAT_DIGIFIX = 0x0016,	/* DSP Solutions' DIGIFIX */
	WAVE_FORMAT_DIALOGIC_OKI_ADPCM = 0x0017,	/* Dialogic OKI ADPCM */
	WAVE_FORMAT_MEDIAVISION_ADPCM = 0x0018,	/* MediaVision ADPCM */
	WAVE_FORMAT_CU_CODEC = 0x0019,	/* HP CU */
	WAVE_FORMAT_YAMAHA_ADPCM = 0x0020,	/* Yamaha ADPCM */
	WAVE_FORMAT_SONARC = 0x0021,	/* Speech Compression's Sonarc */
	WAVE_FORMAT_DSP_TRUESPEECH = 0x0022,	/* DSP Group's True Speech */
	WAVE_FORMAT_ECHOSC1 = 0x0023,	/* Echo Speech's EchoSC1 */
	WAVE_FORMAT_AUDIOFILE_AF36 = 0x0024,	/* Audiofile AF36 */
	WAVE_FORMAT_APTX = 0x0025,	/* APTX */
	WAVE_FORMAT_DOLBY_AC2 = 0x0030,	/* Dolby AC2 */
	WAVE_FORMAT_GSM610 = 0x0031,	/* GSM610 */
	WAVE_FORMAT_MSNAUDIO = 0x0032,	/* MSNAudio */
	WAVE_FORMAT_ANTEX_ADPCME = 0x0033,	/* Antex ADPCME */

	WAVE_FORMAT_MPEG = 0x0050,		/* MPEG */
	WAVE_FORMAT_MPEGLAYER3 = 0x0055,	/* MPEG layer 3 */
	WAVE_FORMAT_LUCENT_G723 = 0x0059,	/* Lucent G.723 */
	WAVE_FORMAT_G726_ADPCM = 0x0064,	/* G.726 ADPCM */
	WAVE_FORMAT_G722_ADPCM = 0x0065,	/* G.722 ADPCM */

	IBM_FORMAT_MULAW = 0x0101,
	IBM_FORMAT_ALAW = 0x0102,
	IBM_FORMAT_ADPCM = 0x0103,

	WAVE_FORMAT_CREATIVE_ADPCM = 0x0200,

	WAVE_FORMAT_EXTENSIBLE = 0xfffe
};

const int _af_wave_compression_types[_AF_WAVE_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW,
	AF_COMPRESSION_IMA,
	AF_COMPRESSION_MS_ADPCM
};

const InstParamInfo _af_wave_inst_params[_AF_WAVE_NUM_INSTPARAMS] =
{
	{ AF_INST_MIDI_BASENOTE, AU_PVTYPE_LONG, "MIDI base note", {60} },
	{ AF_INST_NUMCENTS_DETUNE, AU_PVTYPE_LONG, "Detune in cents", {0} },
	{ AF_INST_MIDI_LOVELOCITY, AU_PVTYPE_LONG, "Low velocity", {1} },
	{ AF_INST_MIDI_HIVELOCITY, AU_PVTYPE_LONG, "High velocity", {127} },
	{ AF_INST_MIDI_LONOTE, AU_PVTYPE_LONG, "Low note", {0} },
	{ AF_INST_MIDI_HINOTE, AU_PVTYPE_LONG, "High note", {127} },
	{ AF_INST_NUMDBS_GAIN, AU_PVTYPE_LONG, "Gain in dB", {0} }
};

static const _AFfilesetup waveDefaultFileSetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_WAVE,		/* fileFormat */
	true,			/* trackSet */
	true,			/* instrumentSet */
	true,			/* miscellaneousSet  */
	1,			/* trackCount */
	NULL,			/* tracks */
	0,			/* instrumentCount */
	NULL,			/* instruments */
	0,			/* miscellaneousCount */
	NULL			/* miscellaneous */
};

static const UUID _af_wave_guid_pcm =
{{
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
}};
static const UUID _af_wave_guid_ieee_float =
{{
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
}};
static const UUID _af_wave_guid_ulaw =
{{
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
}};
static const UUID _af_wave_guid_alaw =
{{
	0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
}};

WAVEFile::WAVEFile()
{
	setFormatByteOrder(AF_BYTEORDER_LITTLEENDIAN);

	m_factOffset = 0;
	m_miscellaneousOffset = 0;
	m_markOffset = 0;
	m_dataSizeOffset = 0;

	m_msadpcmNumCoefficients = 0;
}

status WAVEFile::parseFrameCount(const Tag &id, uint32_t size)
{
	Track *track = getTrack();

	uint32_t totalFrames;
	readU32(&totalFrames);

	track->totalfframes = totalFrames;

	return AF_SUCCEED;
}

status WAVEFile::parseFormat(const Tag &id, uint32_t size)
{
	Track *track = getTrack();

	uint16_t formatTag;
	readU16(&formatTag);
	uint16_t channelCount;
	readU16(&channelCount);
	uint32_t sampleRate;
	readU32(&sampleRate);
	uint32_t averageBytesPerSecond;
	readU32(&averageBytesPerSecond);
	uint16_t blockAlign;
	readU16(&blockAlign);

	if (!channelCount)
	{
		_af_error(AF_BAD_CHANNELS, "invalid file with 0 channels");
		return AF_FAIL;
	}

	track->f.channelCount = channelCount;
	track->f.sampleRate = sampleRate;
	track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;

	/* Default to uncompressed audio data. */
	track->f.compressionType = AF_COMPRESSION_NONE;
	track->f.framesPerPacket = 1;

	switch (formatTag)
	{
		case WAVE_FORMAT_PCM:
		{
			uint16_t bitsPerSample;
			readU16(&bitsPerSample);

			track->f.sampleWidth = bitsPerSample;

			if (bitsPerSample == 0 || bitsPerSample > 32)
			{
				_af_error(AF_BAD_WIDTH,
					"bad sample width of %d bits",
					bitsPerSample);
				return AF_FAIL;
			}

			if (bitsPerSample <= 8)
				track->f.sampleFormat = AF_SAMPFMT_UNSIGNED;
			else
				track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
		}
		break;

		case WAVE_FORMAT_MULAW:
		case IBM_FORMAT_MULAW:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.compressionType = AF_COMPRESSION_G711_ULAW;
			track->f.bytesPerPacket = track->f.channelCount;
			break;

		case WAVE_FORMAT_ALAW:
		case IBM_FORMAT_ALAW:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;
			track->f.compressionType = AF_COMPRESSION_G711_ALAW;
			track->f.bytesPerPacket = track->f.channelCount;
			break;

		case WAVE_FORMAT_IEEE_FLOAT:
		{
			uint16_t bitsPerSample;
			readU16(&bitsPerSample);

			if (bitsPerSample == 64)
			{
				track->f.sampleWidth = 64;
				track->f.sampleFormat = AF_SAMPFMT_DOUBLE;
			}
			else
			{
				track->f.sampleWidth = 32;
				track->f.sampleFormat = AF_SAMPFMT_FLOAT;
			}
		}
		break;

		case WAVE_FORMAT_ADPCM:
		{
			uint16_t bitsPerSample, extraByteCount,
					samplesPerBlock, numCoefficients;

			if (track->f.channelCount != 1 &&
				track->f.channelCount != 2)
			{
				_af_error(AF_BAD_CHANNELS,
					"WAVE file with MS ADPCM compression "
					"must have 1 or 2 channels");
			}

			readU16(&bitsPerSample);
			readU16(&extraByteCount);
			readU16(&samplesPerBlock);
			readU16(&numCoefficients);

			/* numCoefficients should be at least 7. */
			assert(numCoefficients >= 7 && numCoefficients <= 255);

			m_msadpcmNumCoefficients = numCoefficients;

			for (int i=0; i<m_msadpcmNumCoefficients; i++)
			{
				readS16(&m_msadpcmCoefficients[i][0]);
				readS16(&m_msadpcmCoefficients[i][1]);
			}

			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_MS_ADPCM;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;

			track->f.framesPerPacket = samplesPerBlock;
			track->f.bytesPerPacket = blockAlign;

			// Create the parameter list.
			AUpvlist pv = AUpvnew(2);
			AUpvsetparam(pv, 0, _AF_MS_ADPCM_NUM_COEFFICIENTS);
			AUpvsetvaltype(pv, 0, AU_PVTYPE_LONG);
			long l = m_msadpcmNumCoefficients;
			AUpvsetval(pv, 0, &l);

			AUpvsetparam(pv, 1, _AF_MS_ADPCM_COEFFICIENTS);
			AUpvsetvaltype(pv, 1, AU_PVTYPE_PTR);
			void *v = m_msadpcmCoefficients;
			AUpvsetval(pv, 1, &v);

			track->f.compressionParams = pv;
		}
		break;

		case WAVE_FORMAT_DVI_ADPCM:
		{
			uint16_t bitsPerSample, extraByteCount, samplesPerBlock;

			readU16(&bitsPerSample);
			readU16(&extraByteCount);
			readU16(&samplesPerBlock);

			if (bitsPerSample != 4)
			{
				_af_error(AF_BAD_NOT_IMPLEMENTED,
					"IMA ADPCM compression supports only 4 bits per sample");
			}

			int bytesPerBlock = (samplesPerBlock + 14) / 8 * 4 * channelCount;
			if (bytesPerBlock > blockAlign || (samplesPerBlock % 8) != 1)
			{
				_af_error(AF_BAD_CODEC_CONFIG,
					"Invalid samples per block for IMA ADPCM compression");
			}

			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_IMA;
			track->f.byteOrder = _AF_BYTEORDER_NATIVE;

			initIMACompressionParams();

			track->f.framesPerPacket = samplesPerBlock;
			track->f.bytesPerPacket = blockAlign;
		}
		break;

		case WAVE_FORMAT_EXTENSIBLE:
		{
			uint16_t bitsPerSample;
			readU16(&bitsPerSample);
			uint16_t extraByteCount;
			readU16(&extraByteCount);
			uint16_t reserved;
			readU16(&reserved);
			uint32_t channelMask;
			readU32(&channelMask);
			UUID subformat;
			readUUID(&subformat);
			if (subformat == _af_wave_guid_pcm)
			{
				track->f.sampleWidth = bitsPerSample;

				if (bitsPerSample == 0 || bitsPerSample > 32)
				{
					_af_error(AF_BAD_WIDTH,
						"bad sample width of %d bits",
						bitsPerSample);
					return AF_FAIL;
				}

				// Use valid bits per sample if bytes per sample is the same.
				if (reserved <= bitsPerSample &&
					(reserved + 7) / 8 == (bitsPerSample + 7) / 8)
					track->f.sampleWidth = reserved;

				if (bitsPerSample <= 8)
					track->f.sampleFormat = AF_SAMPFMT_UNSIGNED;
				else
					track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			}
			else if (subformat == _af_wave_guid_ieee_float)
			{
				if (bitsPerSample == 64)
				{
					track->f.sampleWidth = 64;
					track->f.sampleFormat = AF_SAMPFMT_DOUBLE;
				}
				else
				{
					track->f.sampleWidth = 32;
					track->f.sampleFormat = AF_SAMPFMT_FLOAT;
				}
			}
			else if (subformat == _af_wave_guid_alaw ||
				subformat == _af_wave_guid_ulaw)
			{
				track->f.compressionType = subformat == _af_wave_guid_alaw ?
					AF_COMPRESSION_G711_ALAW : AF_COMPRESSION_G711_ULAW;
				track->f.sampleWidth = 16;
				track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
				track->f.byteOrder = _AF_BYTEORDER_NATIVE;
				track->f.bytesPerPacket = channelCount;
			}
			else
			{
				_af_error(AF_BAD_NOT_IMPLEMENTED, "WAVE extensible data format %s is not currently supported", subformat.name().c_str());
				return AF_FAIL;
			}
		}
		break;

		case WAVE_FORMAT_YAMAHA_ADPCM:
		case WAVE_FORMAT_OKI_ADPCM:
		case WAVE_FORMAT_CREATIVE_ADPCM:
		case IBM_FORMAT_ADPCM:
			_af_error(AF_BAD_NOT_IMPLEMENTED, "WAVE ADPCM data format 0x%x is not currently supported", formatTag);
			return AF_FAIL;
			break;

		case WAVE_FORMAT_MPEG:
			_af_error(AF_BAD_NOT_IMPLEMENTED, "WAVE MPEG data format is not supported");
			return AF_FAIL;
			break;

		case WAVE_FORMAT_MPEGLAYER3:
			_af_error(AF_BAD_NOT_IMPLEMENTED, "WAVE MPEG layer 3 data format is not supported");
			return AF_FAIL;
			break;

		default:
			_af_error(AF_BAD_NOT_IMPLEMENTED, "WAVE file data format 0x%x not currently supported != 0xfffe ? %d, != EXTENSIBLE? %d", formatTag, formatTag != 0xfffe, formatTag != WAVE_FORMAT_EXTENSIBLE);
			return AF_FAIL;
			break;
	}

	if (track->f.isUncompressed())
		track->f.computeBytesPerPacketPCM();

	_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth);

	return AF_SUCCEED;
}

status WAVEFile::parseData(const Tag &id, uint32_t size)
{
	Track *track = getTrack();

	track->fpos_first_frame = m_fh->tell();
	track->data_size = size;

	return AF_SUCCEED;
}

status WAVEFile::parsePlayList(const Tag &id, uint32_t size)
{
	uint32_t segmentCount;
	readU32(&segmentCount);

	if (segmentCount == 0)
	{
		m_instrumentCount = 0;
		m_instruments = NULL;
		return AF_SUCCEED;
	}

	for (unsigned segment=0; segment<segmentCount; segment++)
	{
		uint32_t startMarkID, loopLength, loopCount;

		readU32(&startMarkID);
		readU32(&loopLength);
		readU32(&loopCount);
	}

	return AF_SUCCEED;
}

status WAVEFile::parseCues(const Tag &id, uint32_t size)
{
	Track *track = getTrack();

	uint32_t markerCount;
	readU32(&markerCount);
	track->markerCount = markerCount;

	if (markerCount == 0)
	{
		track->markers = NULL;
		return AF_SUCCEED;
	}

	if ((track->markers = _af_marker_new(markerCount)) == NULL)
		return AF_FAIL;

	for (unsigned i=0; i<markerCount; i++)
	{
		uint32_t id, position, chunkid;
		uint32_t chunkByteOffset, blockByteOffset;
		uint32_t sampleFrameOffset;
		Marker *marker = &track->markers[i];

		readU32(&id);
		readU32(&position);
		readU32(&chunkid);
		readU32(&chunkByteOffset);
		readU32(&blockByteOffset);

		/*
			sampleFrameOffset represents the position of
			the mark in units of frames.
		*/
		readU32(&sampleFrameOffset);

		marker->id = id;
		marker->position = sampleFrameOffset;
		marker->name = _af_strdup("");
		marker->comment = _af_strdup("");
	}

	return AF_SUCCEED;
}

/* Parse an adtl sub-chunk within a LIST chunk. */
status WAVEFile::parseADTLSubChunk(const Tag &id, uint32_t size)
{
	Track *track = getTrack();

	AFfileoffset endPos = m_fh->tell() + size;

	while (m_fh->tell() < endPos)
	{
		Tag chunkID;
		uint32_t chunkSize;

		readTag(&chunkID);
		readU32(&chunkSize);

		if (chunkID == "labl" || chunkID == "note")
		{
			uint32_t id;
			long length=chunkSize-4;
			char *p = (char *) _af_malloc(length);

			readU32(&id);
			m_fh->read(p, length);

			Marker *marker = track->getMarker(id);

			if (marker)
			{
				if (chunkID == "labl")
				{
					free(marker->name);
					marker->name = p;
				}
				else if (chunkID == "note")
				{
					free(marker->comment);
					marker->comment = p;
				}
				else
					free(p);
			}
			else
				free(p);

			/*
				If chunkSize is odd, skip an extra byte
				at the end of the chunk.
			*/
			if ((chunkSize % 2) != 0)
				m_fh->seek(1, File::SeekFromCurrent);
		}
		else
		{
			/* If chunkSize is odd, skip an extra byte. */
			m_fh->seek(chunkSize + (chunkSize % 2), File::SeekFromCurrent);
		}
	}
	return AF_SUCCEED;
}

/* Parse an INFO sub-chunk within a LIST chunk. */
status WAVEFile::parseINFOSubChunk(const Tag &id, uint32_t size)
{
	AFfileoffset endPos = m_fh->tell() + size;

	while (m_fh->tell() < endPos)
	{
		int misctype = AF_MISC_UNRECOGNIZED;
		Tag miscid;
		uint32_t miscsize;

		readTag(&miscid);
		readU32(&miscsize);

		if (miscid == "IART")
			misctype = AF_MISC_AUTH;
		else if (miscid == "INAM")
			misctype = AF_MISC_NAME;
		else if (miscid == "ICOP")
			misctype = AF_MISC_COPY;
		else if (miscid == "ICMT")
			misctype = AF_MISC_ICMT;
		else if (miscid == "ICRD")
			misctype = AF_MISC_ICRD;
		else if (miscid == "ISFT")
			misctype = AF_MISC_ISFT;

		if (misctype != AF_MISC_UNRECOGNIZED)
		{
			char *string = (char *) _af_malloc(miscsize);

			m_fh->read(string, miscsize);

			m_miscellaneousCount++;
			m_miscellaneous = (Miscellaneous *) _af_realloc(m_miscellaneous, sizeof (Miscellaneous) * m_miscellaneousCount);

			m_miscellaneous[m_miscellaneousCount-1].id = m_miscellaneousCount;
			m_miscellaneous[m_miscellaneousCount-1].type = misctype;
			m_miscellaneous[m_miscellaneousCount-1].size = miscsize;
			m_miscellaneous[m_miscellaneousCount-1].position = 0;
			m_miscellaneous[m_miscellaneousCount-1].buffer = string;
		}
		else
		{
			m_fh->seek(miscsize, File::SeekFromCurrent);
		}

		/* Make the current position an even number of bytes.  */
		if (miscsize % 2 != 0)
			m_fh->seek(1, File::SeekFromCurrent);
	}
	return AF_SUCCEED;
}

status WAVEFile::parseList(const Tag &id, uint32_t size)
{
	Tag typeID;
	readTag(&typeID);
	size-=4;

	if (typeID == "adtl")
	{
		/* Handle adtl sub-chunks. */
		return parseADTLSubChunk(typeID, size);
	}
	else if (typeID == "INFO")
	{
		/* Handle INFO sub-chunks. */
		return parseINFOSubChunk(typeID, size);
	}
	else
	{
		/* Skip unhandled sub-chunks. */
		m_fh->seek(size, File::SeekFromCurrent);
		return AF_SUCCEED;
	}
	return AF_SUCCEED;
}

status WAVEFile::parseInstrument(const Tag &id, uint32_t size)
{
	uint8_t baseNote;
	int8_t detune, gain;
	uint8_t lowNote, highNote, lowVelocity, highVelocity;
	uint8_t padByte;

	readU8(&baseNote);
	readS8(&detune);
	readS8(&gain);
	readU8(&lowNote);
	readU8(&highNote);
	readU8(&lowVelocity);
	readU8(&highVelocity);
	readU8(&padByte);

	return AF_SUCCEED;
}

bool WAVEFile::recognize(File *fh)
{
	uint8_t buffer[8];

	fh->seek(0, File::SeekFromBeginning);

	if (fh->read(buffer, 8) != 8 || memcmp(buffer, "RIFF", 4) != 0)
		return false;
	if (fh->read(buffer, 4) != 4 || memcmp(buffer, "WAVE", 4) != 0)
		return false;

	return true;
}

status WAVEFile::readInit(AFfilesetup setup)
{
	Tag type, formtype;
	uint32_t size;
	uint32_t index = 0;

	bool hasFormat = false;
	bool hasData = false;
	bool hasFrameCount = false;

	Track *track = allocateTrack();

	m_fh->seek(0, File::SeekFromBeginning);

	readTag(&type);
	readU32(&size);
	readTag(&formtype);

	assert(type == "RIFF");
	assert(formtype == "WAVE");

	/* Include the offset of the form type. */
	index += 4;

	while (index < size)
	{
		Tag chunkid;
		uint32_t chunksize = 0;
		status result;

		readTag(&chunkid);
		readU32(&chunksize);

		if (chunkid == "fmt ")
		{
			result = parseFormat(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;

			hasFormat = true;
		}
		else if (chunkid == "data")
		{
			/* The format chunk must precede the data chunk. */
			if (!hasFormat)
			{
				_af_error(AF_BAD_HEADER, "missing format chunk in WAVE file");
				return AF_FAIL;
			}

			result = parseData(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;

			hasData = true;
		}
		else if (chunkid == "inst")
		{
			result = parseInstrument(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkid == "fact")
		{
			hasFrameCount = true;
			result = parseFrameCount(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkid == "cue ")
		{
			result = parseCues(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkid == "LIST" || chunkid == "list")
		{
			result = parseList(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkid == "INST")
		{
			result = parseInstrument(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;
		}
		else if (chunkid == "plst")
		{
			result = parsePlayList(chunkid, chunksize);
			if (result == AF_FAIL)
				return AF_FAIL;
		}

		index += chunksize + 8;

		/* All chunks must be aligned on an even number of bytes */
		if ((index % 2) != 0)
			index++;

		m_fh->seek(index + 8, File::SeekFromBeginning);
	}

	/* The format chunk and the data chunk are required. */
	if (!hasFormat || !hasData)
	{
		return AF_FAIL;
	}

	/*
		At this point we know that the file has a format chunk and a
		data chunk, so we can assume that track->f and track->data_size
		have been initialized.
	*/
	if (!hasFrameCount)
	{
		if (track->f.bytesPerPacket && track->f.framesPerPacket)
		{
			track->computeTotalFileFrames();
		}
		else
		{
			_af_error(AF_BAD_HEADER, "Frame count required but not found");
			return AF_FAIL;
		}
	}

	return AF_SUCCEED;
}

AFfilesetup WAVEFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "WAVE file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();

	if (track->f.isCompressed())
	{
		if (!track->sampleFormatSet)
			_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, 16);
		else
			_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth);
	}
	else if (track->sampleFormatSet)
	{
		switch (track->f.sampleFormat)
		{
			case AF_SAMPFMT_FLOAT:
				if (track->sampleWidthSet &&
					track->f.sampleWidth != 32)
				{
					_af_error(AF_BAD_WIDTH,
						"Warning: invalid sample width for floating-point WAVE file: %d (must be 32 bits)\n",
						track->f.sampleWidth);
					_af_set_sample_format(&track->f, AF_SAMPFMT_FLOAT, 32);
				}
				break;

			case AF_SAMPFMT_DOUBLE:
				if (track->sampleWidthSet &&
					track->f.sampleWidth != 64)
				{
					_af_error(AF_BAD_WIDTH,
						"Warning: invalid sample width for double-precision floating-point WAVE file: %d (must be 64 bits)\n",
						track->f.sampleWidth);
					_af_set_sample_format(&track->f, AF_SAMPFMT_DOUBLE, 64);
				}
				break;

			case AF_SAMPFMT_UNSIGNED:
				if (track->sampleWidthSet)
				{
					if (track->f.sampleWidth < 1 || track->f.sampleWidth > 32)
					{
						_af_error(AF_BAD_WIDTH, "invalid sample width for WAVE file: %d (must be 1-32 bits)\n", track->f.sampleWidth);
						return AF_NULL_FILESETUP;
					}
					if (track->f.sampleWidth > 8)
					{
						_af_error(AF_BAD_SAMPFMT, "WAVE integer data of more than 8 bits must be two's complement signed");
						_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, track->f.sampleWidth);
					}
				}
				else
				/*
					If the sample width is not set but the user requests
					unsigned data, set the width to 8 bits.
				*/
					_af_set_sample_format(&track->f, track->f.sampleFormat, 8);
				break;

			case AF_SAMPFMT_TWOSCOMP:
				if (track->sampleWidthSet)
				{
					if (track->f.sampleWidth < 1 || track->f.sampleWidth > 32)
					{
						_af_error(AF_BAD_WIDTH, "invalid sample width %d for WAVE file (must be 1-32)", track->f.sampleWidth);
						return AF_NULL_FILESETUP;
					}
					else if (track->f.sampleWidth <= 8)
					{
						_af_error(AF_BAD_SAMPFMT, "Warning: WAVE format integer data of 1-8 bits must be unsigned; setting sample format to unsigned");
						_af_set_sample_format(&track->f, AF_SAMPFMT_UNSIGNED, track->f.sampleWidth);
					}
				}
				else
				/*
					If no sample width was specified, we default to 16 bits
					for signed integer data.
				*/
					_af_set_sample_format(&track->f, track->f.sampleFormat, 16);
				break;
		}
	}
	/*
		Otherwise set the sample format depending on the sample
		width or set completely to default.
	*/
	else
	{
		if (!track->sampleWidthSet)
		{
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
		}
		else
		{
			if (track->f.sampleWidth < 1 || track->f.sampleWidth > 32)
			{
				_af_error(AF_BAD_WIDTH, "invalid sample width %d for WAVE file (must be 1-32)", track->f.sampleWidth);
				return AF_NULL_FILESETUP;
			}
			else if (track->f.sampleWidth > 8)
				/* Here track->f.sampleWidth is in {1..32}. */
				track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			else
				/* Here track->f.sampleWidth is in {1..8}. */
				track->f.sampleFormat = AF_SAMPFMT_UNSIGNED;
		}
	}

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW &&
		track->f.compressionType != AF_COMPRESSION_IMA &&
		track->f.compressionType != AF_COMPRESSION_MS_ADPCM)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "compression format not supported in WAVE format");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed() &&
		track->byteOrderSet &&
		track->f.byteOrder != AF_BYTEORDER_LITTLEENDIAN &&
		track->f.isByteOrderSignificant())
	{
		_af_error(AF_BAD_BYTEORDER, "WAVE format only supports little-endian data");
		return AF_NULL_FILESETUP;
	}

	if (track->f.isUncompressed())
		track->f.byteOrder = AF_BYTEORDER_LITTLEENDIAN;

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "WAVE files cannot have AES data");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet)
	{
		if (setup->instrumentCount > 1)
		{
			_af_error(AF_BAD_NUMINSTS, "WAVE files can have 0 or 1 instrument");
			return AF_NULL_FILESETUP;
		}
		else if (setup->instrumentCount == 1)
		{
			if (setup->instruments[0].loopSet &&
				setup->instruments[0].loopCount > 0 &&
				(!track->markersSet || track->markerCount == 0))
			{
				_af_error(AF_BAD_NUMMARKS, "WAVE files with loops must contain at least 1 marker");
				return AF_NULL_FILESETUP;
			}
		}
	}

	/* Make sure the miscellaneous data is of an acceptable type. */
	if (setup->miscellaneousSet)
	{
		for (int i=0; i<setup->miscellaneousCount; i++)
		{
			switch (setup->miscellaneous[i].type)
			{
				case AF_MISC_COPY:
				case AF_MISC_AUTH:
				case AF_MISC_NAME:
				case AF_MISC_ICRD:
				case AF_MISC_ISFT:
				case AF_MISC_ICMT:
					break;
				default:
					_af_error(AF_BAD_MISCTYPE, "illegal miscellaneous type [%d] for WAVE file", setup->miscellaneous[i].type);
					return AF_NULL_FILESETUP;
			}
		}
	}

	/*
		Allocate an AFfilesetup and make all the unset fields correct.
	*/
	AFfilesetup	newsetup = _af_filesetup_copy(setup, &waveDefaultFileSetup, false);

	/* Make sure we do not copy loops if they are not specified in setup. */
	if (setup->instrumentSet && setup->instrumentCount > 0 &&
		setup->instruments[0].loopSet)
	{
		free(newsetup->instruments[0].loops);
		newsetup->instruments[0].loopCount = 0;
	}

	return newsetup;
}

bool WAVEFile::isInstrumentParameterValid(AUpvlist list, int i)
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
			return true;

		default:
			return false;
	}

	return true;
}

status WAVEFile::writeFormat()
{
	uint16_t	formatTag, channelCount;
	uint32_t	sampleRate, averageBytesPerSecond;
	uint16_t	blockAlign;
	uint32_t	chunkSize;
	uint16_t	bitsPerSample;

	Track *track = getTrack();

	m_fh->write("fmt ", 4);

	switch (track->f.compressionType)
	{
		case AF_COMPRESSION_NONE:
			chunkSize = 16;
			if (track->f.sampleFormat == AF_SAMPFMT_FLOAT ||
				track->f.sampleFormat == AF_SAMPFMT_DOUBLE)
			{
				formatTag = WAVE_FORMAT_IEEE_FLOAT;
			}
			else if (track->f.sampleFormat == AF_SAMPFMT_TWOSCOMP ||
				track->f.sampleFormat == AF_SAMPFMT_UNSIGNED)
			{
				formatTag = WAVE_FORMAT_PCM;
			}
			else
			{
				_af_error(AF_BAD_COMPTYPE, "bad sample format");
				return AF_FAIL;
			}

			blockAlign = _af_format_frame_size(&track->f, false);
			bitsPerSample = 8 * _af_format_sample_size(&track->f, false);
			break;

		/*
			G.711 compression uses eight bits per sample.
		*/
		case AF_COMPRESSION_G711_ULAW:
			chunkSize = 18;
			formatTag = IBM_FORMAT_MULAW;
			blockAlign = track->f.channelCount;
			bitsPerSample = 8;
			break;

		case AF_COMPRESSION_G711_ALAW:
			chunkSize = 18;
			formatTag = IBM_FORMAT_ALAW;
			blockAlign = track->f.channelCount;
			bitsPerSample = 8;
			break;

		case AF_COMPRESSION_IMA:
			chunkSize = 20;
			formatTag = WAVE_FORMAT_DVI_ADPCM;
			blockAlign = track->f.bytesPerPacket;
			bitsPerSample = 4;
			break;

		case AF_COMPRESSION_MS_ADPCM:
			chunkSize = 50;
			formatTag = WAVE_FORMAT_ADPCM;
			blockAlign = track->f.bytesPerPacket;
			bitsPerSample = 4;
			break;

		default:
			_af_error(AF_BAD_COMPTYPE, "bad compression type");
			return AF_FAIL;
	}

	writeU32(&chunkSize);
	writeU16(&formatTag);

	channelCount = track->f.channelCount;
	writeU16(&channelCount);

	sampleRate = track->f.sampleRate;
	writeU32(&sampleRate);

	averageBytesPerSecond =
		track->f.sampleRate * _af_format_frame_size(&track->f, false);
	if (track->f.compressionType == AF_COMPRESSION_IMA ||
		track->f.compressionType == AF_COMPRESSION_MS_ADPCM)
		averageBytesPerSecond = track->f.sampleRate * track->f.bytesPerPacket /
			track->f.framesPerPacket;
	writeU32(&averageBytesPerSecond);

	writeU16(&blockAlign);

	writeU16(&bitsPerSample);

	if (track->f.compressionType == AF_COMPRESSION_G711_ULAW ||
		track->f.compressionType == AF_COMPRESSION_G711_ALAW)
	{
		uint16_t zero = 0;
		writeU16(&zero);
	}
	else if (track->f.compressionType == AF_COMPRESSION_IMA)
	{
		uint16_t extraByteCount = 2;
		writeU16(&extraByteCount);
		uint16_t samplesPerBlock = track->f.framesPerPacket;
		writeU16(&samplesPerBlock);
	}
	else if (track->f.compressionType == AF_COMPRESSION_MS_ADPCM)
	{
		uint16_t extraByteCount = 2 + 2 + m_msadpcmNumCoefficients * 4;
		writeU16(&extraByteCount);
		uint16_t samplesPerBlock = track->f.framesPerPacket;
		writeU16(&samplesPerBlock);

		uint16_t numCoefficients = m_msadpcmNumCoefficients;
		writeU16(&numCoefficients);

		for (int i=0; i<m_msadpcmNumCoefficients; i++)
		{
			writeS16(&m_msadpcmCoefficients[i][0]);
			writeS16(&m_msadpcmCoefficients[i][1]);
		}
	}

	return AF_SUCCEED;
}

status WAVEFile::writeFrameCount()
{
	uint32_t factSize = 4;
	uint32_t totalFrameCount;

	Track *track = getTrack();

	/* Omit the fact chunk only for uncompressed integer audio formats. */
	if (track->f.compressionType == AF_COMPRESSION_NONE &&
		(track->f.sampleFormat == AF_SAMPFMT_TWOSCOMP ||
		track->f.sampleFormat == AF_SAMPFMT_UNSIGNED))
		return AF_SUCCEED;

	/*
		If the offset for the fact chunk hasn't been set yet,
		set it to the file's current position.
	*/
	if (m_factOffset == 0)
		m_factOffset = m_fh->tell();
	else
		m_fh->seek(m_factOffset, File::SeekFromBeginning);

	m_fh->write("fact", 4);
	writeU32(&factSize);

	totalFrameCount = track->totalfframes;
	writeU32(&totalFrameCount);

	return AF_SUCCEED;
}

status WAVEFile::writeData()
{
	Track *track = getTrack();

	m_fh->write("data", 4);
	m_dataSizeOffset = m_fh->tell();

	uint32_t chunkSize = track->data_size;

	writeU32(&chunkSize);
	track->fpos_first_frame = m_fh->tell();

	return AF_SUCCEED;
}

status WAVEFile::update()
{
	Track *track = getTrack();

	if (track->fpos_first_frame != 0)
	{
		uint32_t dataLength, fileLength;

		// Update the frame count chunk if present.
		writeFrameCount();

		// Update the length of the data chunk.
		m_fh->seek(m_dataSizeOffset, File::SeekFromBeginning);
		dataLength = (uint32_t) track->data_size;
		writeU32(&dataLength);

		// Update the length of the RIFF chunk.
		fileLength = (uint32_t) m_fh->length();
		fileLength -= 8;

		m_fh->seek(4, File::SeekFromBeginning);
		writeU32(&fileLength);
	}

	/*
		Write the actual data that was set after initializing
		the miscellaneous IDs.	The size of the data will be
		unchanged.
	*/
	writeMiscellaneous();

	// Write the new positions; the size of the data will be unchanged.
	writeCues();

	return AF_SUCCEED;
}

/* Convert an Audio File Library miscellaneous type to a WAVE type. */
static bool misc_type_to_wave (int misctype, Tag *miscid)
{
	if (misctype == AF_MISC_AUTH)
		*miscid = "IART";
	else if (misctype == AF_MISC_NAME)
		*miscid = "INAM";
	else if (misctype == AF_MISC_COPY)
		*miscid = "ICOP";
	else if (misctype == AF_MISC_ICMT)
		*miscid = "ICMT";
	else if (misctype == AF_MISC_ICRD)
		*miscid = "ICRD";
	else if (misctype == AF_MISC_ISFT)
		*miscid = "ISFT";
	else
		return false;

	return true;
}

status WAVEFile::writeMiscellaneous()
{
	if (m_miscellaneousCount != 0)
	{
		uint32_t	miscellaneousBytes;
		uint32_t 	chunkSize;

		/* Start at 12 to account for 'LIST', size, and 'INFO'. */
		miscellaneousBytes = 12;

		/* Then calculate the size of the whole INFO chunk. */
		for (int i=0; i<m_miscellaneousCount; i++)
		{
			Tag miscid;

			// Skip miscellaneous data of an unsupported type.
			if (!misc_type_to_wave(m_miscellaneous[i].type, &miscid))
				continue;

			// Account for miscellaneous type and size.
			miscellaneousBytes += 8;
			miscellaneousBytes += m_miscellaneous[i].size;

			// Add a pad byte if necessary.
			if (m_miscellaneous[i].size % 2 != 0)
				miscellaneousBytes++;

			assert(miscellaneousBytes % 2 == 0);
		}

		if (m_miscellaneousOffset == 0)
			m_miscellaneousOffset = m_fh->tell();
		else
			m_fh->seek(m_miscellaneousOffset, File::SeekFromBeginning);

		/*
			Write the data.  On the first call to this
			function (from _af_wave_write_init), the
			data won't be available, fh->seek is used to
			reserve space until the data has been provided.
			On subseuent calls to this function (from
			_af_wave_update), the data will really be written.
		*/

		/* Write 'LIST'. */
		m_fh->write("LIST", 4);

		/* Write the size of the following chunk. */
		chunkSize = miscellaneousBytes-8;
		writeU32(&chunkSize);

		/* Write 'INFO'. */
		m_fh->write("INFO", 4);

		/* Write each miscellaneous chunk. */
		for (int i=0; i<m_miscellaneousCount; i++)
		{
			uint32_t miscsize = m_miscellaneous[i].size;
			Tag miscid;

			// Skip miscellaneous data of an unsupported type.
			if (!misc_type_to_wave(m_miscellaneous[i].type, &miscid))
				continue;

			writeTag(&miscid);
			writeU32(&miscsize);
			if (m_miscellaneous[i].buffer != NULL)
			{
				uint8_t	zero = 0;

				m_fh->write(m_miscellaneous[i].buffer, m_miscellaneous[i].size);

				// Pad if necessary.
				if ((m_miscellaneous[i].size%2) != 0)
					writeU8(&zero);
			}
			else
			{
				int	size;
				size = m_miscellaneous[i].size;

				// Pad if necessary.
				if ((size % 2) != 0)
					size++;
				m_fh->seek(size, File::SeekFromCurrent);
			}
		}
	}

	return AF_SUCCEED;
}

status WAVEFile::writeCues()
{
	Track *track = getTrack();

	if (!track->markerCount)
		return AF_SUCCEED;

	if (m_markOffset == 0)
		m_markOffset = m_fh->tell();
	else
		m_fh->seek(m_markOffset, File::SeekFromBeginning);

	Tag cue("cue ");
	writeTag(&cue);

	/*
		The cue chunk consists of 4 bytes for the number of cue points
		followed by 24 bytes for each cue point record.
	*/
	uint32_t cueChunkSize = 4 + track->markerCount * 24;
	writeU32(&cueChunkSize);
	uint32_t numCues = track->markerCount;
	writeU32(&numCues);

	// Write each marker to the file.
	for (int i=0; i<track->markerCount; i++)
	{
		uint32_t identifier = track->markers[i].id;
		writeU32(&identifier);

		uint32_t position = i;
		writeU32(&position);

		Tag data("data");
		writeTag(&data);

		/*
			For an uncompressed WAVE file which contains only one data chunk,
			chunkStart and blockStart are zero.
		*/
		uint32_t chunkStart = 0;
		writeU32(&chunkStart);

		uint32_t blockStart = 0;
		writeU32(&blockStart);

		AFframecount markPosition = track->markers[i].position;
		uint32_t sampleOffset = markPosition;
		writeU32(&sampleOffset);
	}

	// Now write the cue names and comments within a master list chunk.
	uint32_t listChunkSize = 4;
	for (int i=0; i<track->markerCount; i++)
	{
		const char *name = track->markers[i].name;
		const char *comment = track->markers[i].comment;

		/*
			Each 'labl' or 'note' chunk consists of 4 bytes for the chunk ID,
			4 bytes for the chunk data size, 4 bytes for the cue point ID,
			and then the length of the label as a null-terminated string.

			In all, this is 12 bytes plus the length of the string, its null
			termination byte, and a trailing pad byte if the length of the
			chunk is otherwise odd.
		*/
		listChunkSize += 12 + zStringLength(name);
		listChunkSize += 12 + zStringLength(comment);
	}

	Tag list("LIST");
	writeTag(&list);
	writeU32(&listChunkSize);
	Tag adtl("adtl");
	writeTag(&adtl);

	for (int i=0; i<track->markerCount; i++)
	{
		uint32_t cuePointID = track->markers[i].id;

		const char *name = track->markers[i].name;
		uint32_t labelSize = 4 + zStringLength(name);
		Tag lablTag("labl");
		writeTag(&lablTag);
		writeU32(&labelSize);
		writeU32(&cuePointID);
		writeZString(name);

		const char *comment = track->markers[i].comment;
		uint32_t noteSize = 4 + zStringLength(comment);
		Tag noteTag("note");
		writeTag(&noteTag);
		writeU32(&noteSize);
		writeU32(&cuePointID);
		writeZString(comment);
	}

	return AF_SUCCEED;
}

bool WAVEFile::writeZString(const char *s)
{
	ssize_t lengthPlusNull = strlen(s) + 1;
	if (m_fh->write(s, lengthPlusNull) != lengthPlusNull)
		return false;
	if (lengthPlusNull & 1)
	{
		uint8_t zero = 0;
		if (!writeU8(&zero))
			return false;
	}
	return true;
}

size_t WAVEFile::zStringLength(const char *s)
{
	size_t lengthPlusNull = strlen(s) + 1;
	return lengthPlusNull + (lengthPlusNull & 1);
}

status WAVEFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	initCompressionParams();

	uint32_t zero = 0;

	m_fh->seek(0, File::SeekFromBeginning);
	m_fh->write("RIFF", 4);
	m_fh->write(&zero, 4);
	m_fh->write("WAVE", 4);

	writeMiscellaneous();
	writeCues();
	writeFormat();
	writeFrameCount();
	writeData();

	return AF_SUCCEED;
}

bool WAVEFile::readUUID(UUID *u)
{
	return m_fh->read(u->data, 16) == 16;
}

bool WAVEFile::writeUUID(const UUID *u)
{
	return m_fh->write(u->data, 16) == 16;
}

void WAVEFile::initCompressionParams()
{
	Track *track = getTrack();
	if (track->f.compressionType == AF_COMPRESSION_IMA)
		initIMACompressionParams();
	else if (track->f.compressionType == AF_COMPRESSION_MS_ADPCM)
		initMSADPCMCompressionParams();
}

void WAVEFile::initIMACompressionParams()
{
	Track *track = getTrack();

	track->f.framesPerPacket = 505;
	track->f.bytesPerPacket = 256 * track->f.channelCount;

	AUpvlist pv = AUpvnew(1);
	AUpvsetparam(pv, 0, _AF_IMA_ADPCM_TYPE);
	AUpvsetvaltype(pv, 0, AU_PVTYPE_LONG);
	long l = _AF_IMA_ADPCM_TYPE_WAVE;
	AUpvsetval(pv, 0, &l);

	track->f.compressionParams = pv;
}

void WAVEFile::initMSADPCMCompressionParams()
{
	const int16_t coefficients[7][2] =
	{
		{ 256, 0 },
		{ 512, -256 },
		{ 0, 0 },
		{ 192, 64 },
		{ 240, 0 },
		{ 460, -208 },
		{ 392, -232 }
	};
	memcpy(m_msadpcmCoefficients, coefficients, sizeof (int16_t) * 7 * 2);
	m_msadpcmNumCoefficients = 7;

	Track *track = getTrack();

	track->f.framesPerPacket = 500;
	track->f.bytesPerPacket = 256 * track->f.channelCount;

	AUpvlist pv = AUpvnew(2);
	AUpvsetparam(pv, 0, _AF_MS_ADPCM_NUM_COEFFICIENTS);
	AUpvsetvaltype(pv, 0, AU_PVTYPE_LONG);
	long l = m_msadpcmNumCoefficients;
	AUpvsetval(pv, 0, &l);

	AUpvsetparam(pv, 1, _AF_MS_ADPCM_COEFFICIENTS);
	AUpvsetvaltype(pv, 1, AU_PVTYPE_PTR);
	void *v = m_msadpcmCoefficients;
	AUpvsetval(pv, 1, &v);

	track->f.compressionParams = pv;
}
