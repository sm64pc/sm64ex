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
#include "Buffer.h"

#include <string.h>

Buffer::Buffer() : m_data(0), m_size(0)
{
}

Buffer::Buffer(size_t size) : m_data(0), m_size(0)
{
	if (size)
		m_data = ::operator new(size);
	if (m_data)
	{
		m_size = size;
	}
}

Buffer::Buffer(const void *data, size_t size) : m_data(0), m_size(0)
{
	if (size)
		m_data = ::operator new(size);
	if (m_data)
	{
		::memcpy(m_data, data, m_size);
		m_size = size;
	}
}

Buffer::~Buffer()
{
	::operator delete(m_data);
}
