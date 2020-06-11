/*
	Audio File Library
	Copyright (C) 2004, Michael Pruett <michael@68k.org>

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
	AVR.h

	This file contains headers and constants related to the AVR
	(Audio Visual Research) sound file format.
*/

#ifndef AVR_H
#define AVR_H

#include "Compiler.h"
#include "FileHandle.h"

class AVRFile : public _AFfilehandle
{
public:
	AVRFile();

	static bool recognize(File *fh);
	static AFfilesetup completeSetup(AFfilesetup);

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;
	status update() OVERRIDE;
};

#endif
