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

#include "config.h"
#include "RebufferModule.h"

#include <algorithm>
#include <assert.h>
#include <string.h>

RebufferModule::RebufferModule(Direction direction, int bytesPerFrame,
	int numFrames, bool multipleOf) :
	m_direction(direction),
	m_bytesPerFrame(bytesPerFrame),
	m_numFrames(numFrames),
	m_multipleOf(multipleOf),
	m_eof(false),
	m_sentShortChunk(false),
	m_buffer(NULL),
	m_offset(-1),
	m_savedBuffer(NULL),
	m_savedOffset(-1)
{
	if (m_direction == FixedToVariable)
		initFixedToVariable();
	else
		initVariableToFixed();
}

RebufferModule::~RebufferModule()
{
	delete [] m_buffer;
	delete [] m_savedBuffer;
}

void RebufferModule::initFixedToVariable()
{
	m_offset = m_numFrames;
	m_buffer = new char[m_numFrames * m_bytesPerFrame];
}

void RebufferModule::initVariableToFixed()
{
	m_offset = 0;
	m_buffer = new char[m_numFrames * m_bytesPerFrame];
	m_savedBuffer = new char[m_numFrames * m_bytesPerFrame];
}

void RebufferModule::maxPull()
{
	assert(m_direction == FixedToVariable);
	if (m_multipleOf)
		m_inChunk->frameCount = m_outChunk->frameCount + m_numFrames;
	else
		m_inChunk->frameCount = m_numFrames;
}

void RebufferModule::maxPush()
{
	assert(m_direction == VariableToFixed);
	if (m_multipleOf)
		m_outChunk->frameCount = m_inChunk->frameCount + m_numFrames;
	else
		m_outChunk->frameCount = m_numFrames;
}

void RebufferModule::runPull()
{
	int framesToPull = m_outChunk->frameCount;
	const char *inBuffer = static_cast<const char *>(m_inChunk->buffer);
	char *outBuffer = static_cast<char *>(m_outChunk->buffer);

	assert(m_offset > 0 && m_offset <= m_numFrames);

	/*
		A module should not pull more frames from its input
		after receiving a short chunk.
	*/
	assert(!m_sentShortChunk);

	if (m_offset < m_numFrames)
	{
		int buffered = m_numFrames - m_offset;
		int n = std::min(framesToPull, buffered);
		memcpy(outBuffer, m_buffer + m_offset * m_bytesPerFrame,
			n * m_bytesPerFrame);
		outBuffer += buffered * m_bytesPerFrame;
		framesToPull -= buffered;
		m_offset += n;
	}

	// Try to pull more frames from the source.
	while (!m_eof && framesToPull > 0)
	{
		int framesRequested;
		if (m_multipleOf)
			// Round framesToPull up to nearest multiple of m_numFrames.
			framesRequested = ((framesToPull - 1) / m_numFrames + 1) * m_numFrames;
		else
			framesRequested = m_numFrames;

		assert(framesRequested > 0);

		pull(framesRequested);

		int framesReceived = m_inChunk->frameCount;

		if (framesReceived != framesRequested)
			m_eof = true;

		memcpy(outBuffer, inBuffer,
			std::min(framesToPull, framesReceived) * m_bytesPerFrame);

		outBuffer += framesReceived * m_bytesPerFrame;
		framesToPull -= framesReceived;

		if (m_multipleOf)
			assert(m_eof || framesToPull <= 0);

		if (framesToPull < 0)
		{
			assert(m_offset == m_numFrames);

			m_offset = m_numFrames + framesToPull;

			assert(m_offset > 0 && m_offset <= m_numFrames);

			memcpy(m_buffer + m_offset * m_bytesPerFrame,
				inBuffer + (framesReceived + framesToPull) * m_bytesPerFrame,
				(m_numFrames - m_offset) * m_bytesPerFrame);
		}
		else
		{
			assert(m_offset == m_numFrames);
		}
	}

	if (m_eof && framesToPull > 0)
	{
		// Output short chunk.
		m_outChunk->frameCount -= framesToPull;
		m_sentShortChunk = true;
		assert(m_offset == m_numFrames);
	}
	else
	{
		assert(framesToPull <= 0);
		assert(m_offset == m_numFrames + framesToPull);
	}
	assert(m_offset > 0 && m_offset <= m_numFrames);
}

void RebufferModule::reset1()
{
	m_offset = m_numFrames;
	m_eof = false;
	m_sentShortChunk = false;
	assert(m_offset > 0 && m_offset <= m_numFrames);
}

void RebufferModule::reset2()
{
	assert(m_offset > 0 && m_offset <= m_numFrames);
}

void RebufferModule::runPush()
{
	int framesToPush = m_inChunk->frameCount;
	const char *inBuffer = static_cast<const char *>(m_inChunk->buffer);
	char *outBuffer = static_cast<char *>(m_outChunk->buffer);

	assert(m_offset >= 0 && m_offset < m_numFrames);

	// Check that we will be able to push even one block.
	if (m_offset + framesToPush >= m_numFrames)
	{
		if (m_offset > 0)
			memcpy(m_outChunk->buffer, m_buffer, m_offset * m_bytesPerFrame);

		if (m_multipleOf)
		{
			// Round down to nearest multiple of m_numFrames.
			int n = ((m_offset + framesToPush) / m_numFrames) * m_numFrames;

			assert(n > m_offset);
			memcpy(outBuffer + m_offset * m_bytesPerFrame,
				inBuffer,
				(n - m_offset) * m_bytesPerFrame);

			push(n);

			inBuffer += (n - m_offset) * m_bytesPerFrame;
			framesToPush -= n - m_offset;
			assert(framesToPush >= 0);
			m_offset = 0;
		}
		else
		{
			while (m_offset + framesToPush >= m_numFrames)
			{
				int n = m_numFrames - m_offset;
				memcpy(outBuffer + m_offset * m_bytesPerFrame,
					inBuffer,
					n * m_bytesPerFrame);

				push(m_numFrames);

				inBuffer += n * m_bytesPerFrame;
				framesToPush -= n;
				assert(framesToPush >= 0);
				m_offset = 0;
			}
		}

		assert(m_offset == 0);
	}

	assert(m_offset + framesToPush < m_numFrames);

	// Save remaining samples in buffer.
	if (framesToPush > 0)
	{
		memcpy(m_buffer + m_offset * m_bytesPerFrame,
			inBuffer,
			framesToPush * m_bytesPerFrame);
		m_offset += framesToPush;
	}

	assert(m_offset >= 0 && m_offset < m_numFrames);
}

void RebufferModule::sync1()
{
	assert(m_offset >= 0 && m_offset < m_numFrames);

	// Save all the frames and the offset so we can restore our state later.
	memcpy(m_savedBuffer, m_buffer, m_numFrames * m_bytesPerFrame);
	m_savedOffset = m_offset;
}

void RebufferModule::sync2()
{
	assert(m_offset >= 0 && m_offset < m_numFrames);

	memcpy(m_outChunk->buffer, m_buffer, m_offset * m_bytesPerFrame);

	push(m_offset);

	memcpy(m_buffer, m_savedBuffer, m_numFrames * m_bytesPerFrame);
	m_offset = m_savedOffset;

	assert(m_offset >= 0 && m_offset < m_numFrames);
}
