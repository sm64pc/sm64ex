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

#ifndef FLACFile_h
#define FLACFile_h

#include "Compiler.h"
#include "FileHandle.h"
#include "Features.h"

#if ENABLE(FLAC)
#include <FLAC/format.h>
#include <FLAC/stream_decoder.h>
#endif

#define _AF_FLAC_NUM_COMPTYPES 1
extern const int _af_flac_compression_types[_AF_FLAC_NUM_COMPTYPES];

class FLACFile : public _AFfilehandle
{
public:
	static bool recognize(File *);
	static AFfilesetup completeSetup(AFfilesetup);

	FLACFile();
	~FLACFile();

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;
	status update() OVERRIDE;

private:
#if ENABLE(FLAC)
	void parseStreamInfo(const FLAC__StreamMetadata_StreamInfo &);

	static FLAC__StreamDecoderReadStatus readCallback(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *clientData);
	static FLAC__StreamDecoderSeekStatus seekCallback(const FLAC__StreamDecoder *, FLAC__uint64 absoluteByteOffset, void *clientData);
	static FLAC__StreamDecoderTellStatus tellCallback(const FLAC__StreamDecoder *, FLAC__uint64 *absoluteByteOffset, void *clientData);
	static FLAC__StreamDecoderLengthStatus lengthCallback(const FLAC__StreamDecoder *, FLAC__uint64 *length, void *clientData);
	static FLAC__bool eofCallback(const FLAC__StreamDecoder *, void *clientData);
	static FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData);
	static void metadataCallback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *clientData);
	static void errorCallback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *clientData);
#endif
};

#endif
