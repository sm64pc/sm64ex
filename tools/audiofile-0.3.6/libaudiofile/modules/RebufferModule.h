/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.
	Copyright (C) 2010, Michael Pruett <michael@68k.org>

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

#ifndef REBUFFER_MODULE_H
#define REBUFFER_MODULE_H

#include "Compiler.h"
#include "Module.h"

class RebufferModule : public Module
{
public:
	enum Direction
	{
		FixedToVariable,
		VariableToFixed
	};

	RebufferModule(Direction, int bytesPerFrame, int numFrames, bool multipleOf);
	virtual ~RebufferModule();

	virtual const char *name() const OVERRIDE { return "rebuffer"; }

	virtual void maxPull() OVERRIDE;
	virtual void maxPush() OVERRIDE;

	virtual void runPull() OVERRIDE;
	virtual void reset1() OVERRIDE;
	virtual void reset2() OVERRIDE;
	virtual void runPush() OVERRIDE;
	virtual void sync1() OVERRIDE;
	virtual void sync2() OVERRIDE;

private:
	Direction m_direction;
	int m_bytesPerFrame;
	int m_numFrames;
	bool m_multipleOf; // buffer to multiple of m_numFrames
	bool m_eof; // end of input stream reached
	bool m_sentShortChunk; // end of input stream indicated
	char *m_buffer;
	int m_offset;
	char *m_savedBuffer;
	int m_savedOffset;

	void initFixedToVariable();
	void initVariableToFixed();
};

#endif // REBUFFER_MODULE_H
