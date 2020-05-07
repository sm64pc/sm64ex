/*
	Audio File Library
	Copyright (C) 2012, Michael Pruett <michael@68k.org>

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

#ifndef SAMPLE_VISION_H
#define SAMPLE_VISION_H

#include "Compiler.h"
#include "FileHandle.h"

class SampleVisionFile : public _AFfilehandle
{
public:
	SampleVisionFile();
	virtual ~SampleVisionFile();

	static bool recognize(File *fh);

	static AFfilesetup completeSetup(AFfilesetup);

	status readInit(AFfilesetup) OVERRIDE;
	status writeInit(AFfilesetup) OVERRIDE;

	status update() OVERRIDE;

private:
	AFfileoffset m_frameCountOffset;

	status parseLoops();
	status parseMarkers();
	status writeTrailer();
	status writeLoops();
	status writeMarkers();

	void addMiscellaneous(int type, const char *data);
};

#endif
