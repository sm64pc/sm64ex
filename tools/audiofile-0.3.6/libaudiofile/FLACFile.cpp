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
#include "FLACFile.h"

#include "File.h"
#include "Setup.h"
#include "Track.h"
#include "byteorder.h"
#include "util.h"

#include <string.h>

const int _af_flac_compression_types[_AF_FLAC_NUM_COMPTYPES] =
{
	AF_COMPRESSION_FLAC
};

const _AFfilesetup flacDefaultFileSetup =
{
	_AF_VALID_FILESETUP,
	AF_FILE_FLAC,
	true,
	true,
	true,
	1,		// trackCount
	NULL,	// tracks
	0,		// instrumentCount
	NULL,	// instruments
	0,		// miscellaneousCount
	NULL	// miscellaneous
};

bool FLACFile::recognize(File *file)
{
	file->seek(0, File::SeekFromBeginning);
	uint8_t buffer[4];
	if (file->read(buffer, 4) != 4 || memcmp(buffer, "fLaC", 4) != 0)
		return false;
	return true;
}

FLACFile::FLACFile()
{
}

FLACFile::~FLACFile()
{
}

#if ENABLE(FLAC)

AFfilesetup FLACFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "FLAC file must have 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();

	if (track->sampleFormatSet &&
		track->f.sampleFormat != AF_SAMPFMT_TWOSCOMP)
	{
		_af_error(AF_BAD_SAMPFMT,
			"FLAC files support only signed integer audio data");
		return AF_NULL_FILESETUP;
	}

	if (track->sampleWidthSet &&
		track->f.sampleWidth != 16 && track->f.sampleWidth != 24)
	{
		_af_error(AF_BAD_WIDTH,
			"FLAC files support only 16- or 24-bit audio data");
		return AF_NULL_FILESETUP;
	}

	_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, track->f.sampleWidth);

	track->f.byteOrder = _AF_BYTEORDER_NATIVE;

	if (track->compressionSet &&
		track->f.compressionType != AF_COMPRESSION_FLAC)
	{
		_af_error(AF_BAD_COMPTYPE, "Only FLAC compression supported in FLAC files");
		return AF_NULL_FILESETUP;
	}

	track->f.compressionType = AF_COMPRESSION_FLAC;

	if (track->markersSet && track->markerCount)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "FLAC does not yet support markers");
		return AF_NULL_FILESETUP;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "FLAC does not support AES data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &flacDefaultFileSetup, true);
}

status FLACFile::readInit(AFfilesetup)
{
	m_fh->seek(0, File::SeekFromBeginning);

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();

	if (FLAC__stream_decoder_init_stream(decoder,
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
		_af_error(AF_BAD_HEADER, "could not read FLAC file");
		return AF_FAIL;
	}

	if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder))
		return AF_FAIL;

	FLAC__uint64 position;
	if (!FLAC__stream_decoder_get_decode_position(decoder, &position))
		return AF_FAIL;

	Track *track = getTrack();
	if (track)
	{
		track->fpos_first_frame = static_cast<off_t>(position);
		track->data_size = m_fh->length() - track->fpos_first_frame;
	}

	FLAC__stream_decoder_delete(decoder);

	if (!track)
		return AF_FAIL;
	return AF_SUCCEED;
}

status FLACFile::writeInit(AFfilesetup setup)
{
	if (initFromSetup(setup) == AF_FAIL)
		return AF_FAIL;

	return AF_SUCCEED;
}

status FLACFile::update()
{
	return AF_SUCCEED;
}

void FLACFile::parseStreamInfo(const FLAC__StreamMetadata_StreamInfo &streamInfo)
{
	Track *track = allocateTrack();

	track->f.channelCount = streamInfo.channels;
	track->f.sampleRate = streamInfo.sample_rate;

	track->f.byteOrder = _AF_BYTEORDER_NATIVE;

	track->f.framesPerPacket = 0;
	track->f.bytesPerPacket = 0;

	track->f.compressionType = AF_COMPRESSION_FLAC;
	track->f.compressionParams = NULL;

	_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, streamInfo.bits_per_sample);

	track->totalfframes = streamInfo.total_samples;
}

FLAC__StreamDecoderReadStatus FLACFile::readCallback(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *clientData)
{
	FLACFile *flac = static_cast<FLACFile *>(clientData);
	ssize_t result = flac->m_fh->read(buffer, *bytes);
	if (result > 0)
	{
		*bytes = result;
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}

	*bytes = 0;
	return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

FLAC__StreamDecoderSeekStatus FLACFile::seekCallback(const FLAC__StreamDecoder *, FLAC__uint64 absoluteByteOffset, void *clientData)
{
	FLACFile *flac = static_cast<FLACFile *>(clientData);
	if (flac->m_fh->seek(static_cast<off_t>(absoluteByteOffset), File::SeekFromBeginning) < 0)
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus FLACFile::tellCallback(const FLAC__StreamDecoder *, FLAC__uint64 *absoluteByteOffset, void *clientData)
{
	FLACFile *flac = static_cast<FLACFile *>(clientData);
	off_t result = flac->m_fh->tell();
	if (result < 0)
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	*absoluteByteOffset = static_cast<FLAC__uint64>(result);
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus FLACFile::lengthCallback(const FLAC__StreamDecoder *, FLAC__uint64 *length, void *clientData)
{
	FLACFile *flac = static_cast<FLACFile *>(clientData);
	off_t result = flac->m_fh->length();
	if (result < 0)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	*length = result;
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool FLACFile::eofCallback(const FLAC__StreamDecoder *, void *clientData)
{
	FLACFile *flac = static_cast<FLACFile *>(clientData);
	return flac->m_fh->tell() == flac->m_fh->length();
}

FLAC__StreamDecoderWriteStatus FLACFile::writeCallback(const FLAC__StreamDecoder *, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData)
{
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FLACFile::metadataCallback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *clientData)
{
	FLACFile *flac = static_cast<FLACFile *>(clientData);
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
		flac->parseStreamInfo(metadata->data.stream_info);
}

void FLACFile::errorCallback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *clientData)
{
	_af_error(AF_BAD_FILEFMT, "error %d parsing FLAC file", status);
}

#else

AFfilesetup FLACFile::completeSetup(AFfilesetup)
{
	_af_error(AF_BAD_FILEFMT, "FLAC is disabled");
	return AF_NULL_FILESETUP;
}

status FLACFile::readInit(AFfilesetup)
{
	_af_error(AF_BAD_FILEFMT, "FLAC is disabled");
	return AF_FAIL;
}

status FLACFile::writeInit(AFfilesetup)
{
	return AF_FAIL;
}

status FLACFile::update()
{
	return AF_FAIL;
}

#endif
