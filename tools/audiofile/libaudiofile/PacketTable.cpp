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
#include "PacketTable.h"

PacketTable::PacketTable(int64_t numValidFrames, int32_t primingFrames,
	int32_t remainderFrames) :
	m_numValidFrames(numValidFrames),
	m_primingFrames(primingFrames),
	m_remainderFrames(remainderFrames)
{
}

PacketTable::PacketTable()
{
	m_numValidFrames = 0;
	m_primingFrames = 0;
	m_remainderFrames = 0;
}

PacketTable::~PacketTable()
{
}

void PacketTable::setNumValidFrames(int64_t numValidFrames)
{
	m_numValidFrames = numValidFrames;
}

void PacketTable::setPrimingFrames(int32_t primingFrames)
{
	m_primingFrames = primingFrames;
}

void PacketTable::setRemainderFrames(int32_t remainderFrames)
{
	m_remainderFrames = remainderFrames;
}

void PacketTable::append(size_t bytesPerPacket)
{
	m_bytesPerPacket.push_back(bytesPerPacket);
}

AFfileoffset PacketTable::startOfPacket(size_t packet) const
{
	AFfileoffset offset = 0;
	for (size_t i=0; i<packet; i++)
		offset += m_bytesPerPacket[i];
	return offset;
}
