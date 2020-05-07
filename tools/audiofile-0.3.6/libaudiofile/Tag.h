/*
	Audio File Library
	Copyright (C) 2011, Michael Pruett <michael@68k.org>

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

#ifndef TAG_H
#define TAG_H

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <string>

class Tag
{
public:
	Tag() : m_value(0) { }
	Tag(uint32_t value) : m_value(value) { }
	Tag(const char *s)
	{
		assert(strlen(s) == 4);
		memcpy(&m_value, s, 4);
	}

	uint32_t value() const { return m_value; }

	bool operator==(const Tag &t) const { return m_value == t.m_value; }
	bool operator!=(const Tag &t) const { return m_value != t.m_value; }

	std::string name() const
	{
		char s[5];
		memcpy(s, &m_value, 4);
		s[4] = '\0';
		return std::string(s);
	}

private:
	uint32_t m_value;
};

#endif
