/*
	Audio File Library
	Copyright (C) 2011-2013 Michael Pruett <michael@68k.org>

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

#ifndef CAF_H
#define CAF_H

#include "Compiler.h"
#include "File.h"
#include "FileHandle.h"
#include "Tag.h"
#include <stdint.h>

#define _AF_CAF_NUM_COMPTYPES 4
extern const int _af_caf_compression_types[_AF_CAF_NUM_COMPTYPES];

class Buffer;

class CAFFile : public _AFfilehandle
{
public:
	static bool recognize(File *);
	static AFfilesetup completeSetup(AFfilesetup);

	CAFFile();
	~CAFFile();

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;
	status update() OVERRIDE;

private:
	AFfileoffset m_dataOffset;
	AFfileoffset m_cookieDataOffset;
	SharedPtr<Buffer> m_codecData;

	status parseDescription(const Tag &, int64_t);
	status parseData(const Tag &, int64_t);
	status parsePacketTable(const Tag &, int64_t);
	status parseCookieData(const Tag &, int64_t);

	status writeDescription();
	status writeData(bool update);
	status writePacketTable();
	status writeCookieData();

	void initCompressionParams();
	void initIMACompressionParams();
	void initALACCompressionParams();
};

#endif
