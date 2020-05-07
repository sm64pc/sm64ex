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

#ifndef MODULE_H
#define MODULE_H

#include "AudioFormat.h"
#include "Shared.h"
#include "afinternal.h"

#include <vector>

enum FormatCode
{
	kUndefined = -1,
	kInt8,
	kInt16,
	kInt24,
	kInt32,
	kFloat,
	kDouble,
};

class Chunk : public Shared<Chunk>
{
public:
	void *buffer;
	size_t frameCount;
	AudioFormat f;
	bool ownsMemory;

	Chunk() : buffer(NULL), frameCount(0), ownsMemory(false) { }
	~Chunk()
	{
		deallocate();
	}
	void allocate(size_t capacity)
	{
		deallocate();
		ownsMemory = true;
		buffer = ::operator new(capacity);
	}
	void deallocate()
	{
		if (ownsMemory)
			::operator delete(buffer);
		ownsMemory = false;
		buffer = NULL;
	}
};

class Module : public Shared<Module>
{
public:
	Module();
	virtual ~Module();

	void setSink(Module *);
	void setSource(Module *);
	Chunk *inChunk() const { return m_inChunk.get(); }
	void setInChunk(Chunk *chunk) { m_inChunk = chunk; }
	Chunk *outChunk() const { return m_outChunk.get(); }
	void setOutChunk(Chunk *chunk) { m_outChunk = chunk; }

	virtual const char *name() const;
	/*
		Set format of m_outChunk based on how this module transforms m_inChunk.
	*/
	virtual void describe();
	/*
		Set frame count of m_inChunk to the maximum number of frames needed to
		produce frame count of m_outChunk.
	*/
	virtual void maxPull();
	/*
		Set frame count of m_outChunk to the maximum number of frames needed to
		produce frame count of m_inChunk.
	*/
	virtual void maxPush();
	virtual void runPull();
	virtual void reset1() { }
	virtual void reset2() { }
	virtual void runPush();
	virtual void sync1() { }
	virtual void sync2() { }

protected:
	SharedPtr<Chunk> m_inChunk, m_outChunk;
	union
	{
		Module *m_sink;
		Module *m_source;
	};

	void pull(size_t frames);
	void push(size_t frames);
};

/*
	_AF_ATOMIC_NVFRAMES is NOT the maximum number of frames a module
	can be requested to produce.

	This IS the maximum number of virtual (user) frames that will
	be produced or processed per run of the modules.

	Modules can be requested more frames than this because of rate
	conversion and rebuffering.
*/

#define _AF_ATOMIC_NVFRAMES 1024

#endif // MODULE_H
