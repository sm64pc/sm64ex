/*
	Audio File Library
	Copyright (C) 2011, Michael Pruett <michael@68k.org>

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

#ifndef VOC_H
#define VOC_H

#include "Compiler.h"
#include "FileHandle.h"

#define _AF_VOC_NUM_COMPTYPES 2
extern const int _af_voc_compression_types[_AF_VOC_NUM_COMPTYPES];

class VOCFile : public _AFfilehandle
{
public:
	VOCFile();

	static bool recognize(File *);
	static AFfilesetup completeSetup(AFfilesetup);

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;
	status update() OVERRIDE;

private:
	status writeSoundData();
	status writeTerminator();

	AFfileoffset m_soundDataOffset;
};

#endif
