/*
	Audio File Library
	Copyright (C) 1998-2000, 2003, 2010-2012, Michael Pruett <michael@68k.org>
	Copyright (C) 2002-2003, Davy Durham
	Copyright (C) 2000-2001, Silicon Graphics, Inc.

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
	WAVE.h

	This file contains structures and constants related to the RIFF
	WAVE sound file format.
*/

#ifndef WAVE_H
#define WAVE_H

#include "Compiler.h"
#include "FileHandle.h"
#include <stdint.h>

#define _AF_WAVE_NUM_INSTPARAMS 7
extern const InstParamInfo _af_wave_inst_params[_AF_WAVE_NUM_INSTPARAMS];
#define _AF_WAVE_NUM_COMPTYPES 4
extern const int _af_wave_compression_types[_AF_WAVE_NUM_COMPTYPES];

struct UUID;

class WAVEFile : public _AFfilehandle
{
public:
	static bool recognize(File *fh);
	static AFfilesetup completeSetup(AFfilesetup);

	WAVEFile();

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;

	status update() OVERRIDE;

	bool isInstrumentParameterValid(AUpvlist, int) OVERRIDE;

private:
	AFfileoffset m_factOffset;	// start of fact (frame count) chunk
	AFfileoffset m_miscellaneousOffset;
	AFfileoffset m_markOffset;
	AFfileoffset m_dataSizeOffset;

	/*
		The index into the coefficient array is of type
		uint8_t, so we can safely limit msadpcmCoefficients to
		be 256 coefficient pairs.
	*/
	int m_msadpcmNumCoefficients;
	int16_t m_msadpcmCoefficients[256][2];

	status parseFrameCount(const Tag &type, uint32_t size);
	status parseFormat(const Tag &type, uint32_t size);
	status parseData(const Tag &type, uint32_t size);
	status parsePlayList(const Tag &type, uint32_t size);
	status parseCues(const Tag &type, uint32_t size);
	status parseADTLSubChunk(const Tag &type, uint32_t size);
	status parseINFOSubChunk(const Tag &type, uint32_t size);
	status parseList(const Tag &type, uint32_t size);
	status parseInstrument(const Tag &type, uint32_t size);

	status writeFormat();
	status writeFrameCount();
	status writeMiscellaneous();
	status writeCues();
	status writeData();

	bool readUUID(UUID *g);
	bool writeUUID(const UUID *g);

	bool writeZString(const char *);
	size_t zStringLength(const char *);

	void initCompressionParams();
	void initIMACompressionParams();
	void initMSADPCMCompressionParams();
};

#endif
