/*
	Audio File Library
	Copyright (C) 1998-2000, 2003-2004, 2010-2012, Michael Pruett <michael@68k.org>

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
	AIFF.h

	This file contains structures and constants related to the AIFF
	and AIFF-C formats.
*/

#ifndef AIFF_H
#define AIFF_H

#include "Compiler.h"
#include "FileHandle.h"

#define _AF_AIFF_NUM_INSTPARAMS 9
extern const InstParamInfo _af_aiff_inst_params[_AF_AIFF_NUM_INSTPARAMS];
#define _AF_AIFFC_NUM_COMPTYPES 3
extern const int _af_aiffc_compression_types[_AF_AIFFC_NUM_COMPTYPES];

class AIFFFile : public _AFfilehandle
{
public:
	AIFFFile();

	static bool recognizeAIFF(File *fh);
	static bool recognizeAIFFC(File *fh);

	static AFfilesetup completeSetup(AFfilesetup);

	int getVersion() OVERRIDE;

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;

	status update() OVERRIDE;

	bool isInstrumentParameterValid(AUpvlist, int) OVERRIDE;

private:
	AFfileoffset m_miscellaneousPosition;
	AFfileoffset m_FVER_offset;
	AFfileoffset m_COMM_offset;
	AFfileoffset m_MARK_offset;
	AFfileoffset m_INST_offset;
	AFfileoffset m_AESD_offset;
	AFfileoffset m_SSND_offset;

	status parseFVER(const Tag &type, size_t size);
	status parseAESD(const Tag &type, size_t size);
	status parseMiscellaneous(const Tag &type, size_t size);
	status parseINST(const Tag &type, size_t size);
	status parseMARK(const Tag &type, size_t size);
	status parseCOMM(const Tag &type, size_t size);
	status parseSSND(const Tag &type, size_t size);

	status writeCOMM();
	status writeSSND();
	status writeMARK();
	status writeINST();
	status writeFVER();
	status writeAESD();
	status writeMiscellaneous();

	void initCompressionParams();
	void initIMACompressionParams();

	bool isAIFFC() const { return m_fileFormat == AF_FILE_AIFFC; }

	bool readPString(char s[256]);
	bool writePString(const char *);
};

#endif
