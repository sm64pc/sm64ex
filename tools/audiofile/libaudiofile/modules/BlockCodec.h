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

// BlockCodec is a base class for codecs with fixed-size packets.

#ifndef BlockCodec_h
#define BlockCodec_h

#include "Compiler.h"
#include "FileModule.h"

class BlockCodec : public FileModule
{
public:
	virtual void runPull() OVERRIDE;
	virtual void reset1() OVERRIDE;
	virtual void reset2() OVERRIDE;
	virtual void runPush() OVERRIDE;
	virtual void sync1() OVERRIDE;
	virtual void sync2() OVERRIDE;

protected:
	int m_bytesPerPacket, m_framesPerPacket;
	AFframecount m_framesToIgnore;
	AFfileoffset m_savedPositionNextFrame;
	AFframecount m_savedNextFrame;

	BlockCodec(Mode, Track *, File *, bool canSeek);

	virtual int decodeBlock(const uint8_t *encoded, int16_t *decoded) = 0;
	virtual int encodeBlock(const int16_t *decoded, uint8_t *encoded) = 0;
};

#endif
