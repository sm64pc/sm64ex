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

#include "config.h"
#include "BlockCodec.h"

#include "Track.h"

#include <assert.h>

BlockCodec::BlockCodec(Mode mode, Track *track, File *fh, bool canSeek) :
	FileModule(mode, track, fh, canSeek),
	m_bytesPerPacket(-1),
	m_framesPerPacket(-1),
	m_framesToIgnore(-1),
	m_savedPositionNextFrame(-1),
	m_savedNextFrame(-1)
{
	m_framesPerPacket = track->f.framesPerPacket;
	m_bytesPerPacket = track->f.bytesPerPacket;
}

void BlockCodec::runPull()
{
	AFframecount framesToRead = m_outChunk->frameCount;
	AFframecount framesRead = 0;

	assert(framesToRead % m_framesPerPacket == 0);
	int blockCount = framesToRead / m_framesPerPacket;

	// Read the compressed data.
	ssize_t bytesRead = read(m_inChunk->buffer, m_bytesPerPacket * blockCount);
	int blocksRead = bytesRead >= 0 ? bytesRead / m_bytesPerPacket : 0;

	// Decompress into m_outChunk.
	for (int i=0; i<blocksRead; i++)
	{
		decodeBlock(static_cast<const uint8_t *>(m_inChunk->buffer) + i * m_bytesPerPacket,
			static_cast<int16_t *>(m_outChunk->buffer) + i * m_framesPerPacket * m_track->f.channelCount);

		framesRead += m_framesPerPacket;
	}

	m_track->nextfframe += framesRead;

	assert(tell() == m_track->fpos_next_frame);

	if (framesRead < framesToRead)
		reportReadError(framesRead, framesToRead);

	m_outChunk->frameCount = framesRead;
}

void BlockCodec::reset1()
{
	AFframecount nextTrackFrame = m_track->nextfframe;
	m_track->nextfframe = (nextTrackFrame / m_framesPerPacket) *
		m_framesPerPacket;

	m_framesToIgnore = nextTrackFrame - m_track->nextfframe;
}

void BlockCodec::reset2()
{
	m_track->fpos_next_frame = m_track->fpos_first_frame +
		m_bytesPerPacket * (m_track->nextfframe / m_framesPerPacket);
	m_track->frames2ignore += m_framesToIgnore;

	assert(m_track->nextfframe % m_framesPerPacket == 0);
}

void BlockCodec::runPush()
{
	AFframecount framesToWrite = m_inChunk->frameCount;
	int channelCount = m_inChunk->f.channelCount;

	int blockCount = (framesToWrite + m_framesPerPacket - 1) / m_framesPerPacket;
	for (int i=0; i<blockCount; i++)
	{
		encodeBlock(static_cast<const int16_t *>(m_inChunk->buffer) + i * m_framesPerPacket * channelCount,
			static_cast<uint8_t *>(m_outChunk->buffer) + i * m_bytesPerPacket);
	}

	ssize_t bytesWritten = write(m_outChunk->buffer, m_bytesPerPacket * blockCount);
	ssize_t blocksWritten = bytesWritten >= 0 ? bytesWritten / m_bytesPerPacket : 0;
	AFframecount framesWritten = std::min((AFframecount) blocksWritten * m_framesPerPacket, framesToWrite);

	m_track->nextfframe += framesWritten;
	m_track->totalfframes = m_track->nextfframe;

	assert(tell() == m_track->fpos_next_frame);

	if (framesWritten < framesToWrite)
		reportWriteError(framesWritten, framesToWrite);
}

void BlockCodec::sync1()
{
	m_savedPositionNextFrame = m_track->fpos_next_frame;
	m_savedNextFrame = m_track->nextfframe;
}

void BlockCodec::sync2()
{
	assert(tell() == m_track->fpos_next_frame);
	m_track->fpos_after_data = tell();
	m_track->fpos_next_frame = m_savedPositionNextFrame;
	m_track->nextfframe = m_savedNextFrame;
}
