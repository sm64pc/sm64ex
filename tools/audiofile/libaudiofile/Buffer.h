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

#ifndef Buffer_h
#define Buffer_h

#include "Shared.h"

#include <sys/types.h>

class Buffer : public Shared<Buffer>
{
public:
	Buffer();
	Buffer(size_t size);
	Buffer(const void *data, size_t size);
	~Buffer();

	void *data() { return m_data; }
	const void *data() const { return m_data; }

	size_t size() const { return m_size; }

private:
	void *m_data;
	size_t m_size;
};

#endif
