/*
	Audio File Library
	Copyright (C) 1998-1999, 2010-2011, Michael Pruett <michael@68k.org>

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

#ifndef BYTEORDER_H
#define BYTEORDER_H

#include "config.h"

#include <stdint.h>

#if WORDS_BIGENDIAN
	#define _AF_BYTEORDER_NATIVE (AF_BYTEORDER_BIGENDIAN)
#else
	#define _AF_BYTEORDER_NATIVE (AF_BYTEORDER_LITTLEENDIAN)
#endif

inline uint16_t _af_byteswap_int16 (uint16_t x)
{
	return (x >> 8) | (x << 8);
}

inline uint32_t _af_byteswap_int32 (uint32_t x)
{
	return ((x & 0x000000ffU) << 24) |
		((x & 0x0000ff00U) << 8) |
		((x & 0x00ff0000U) >> 8) |
		((x & 0xff000000U) >> 24);
}

inline uint64_t _af_byteswap_int64 (uint64_t x)
{
	return ((x & 0x00000000000000ffULL) << 56) |
		((x & 0x000000000000ff00ULL) << 40) |
		((x & 0x0000000000ff0000ULL) << 24) |
		((x & 0x00000000ff000000ULL) << 8) |
		((x & 0x000000ff00000000ULL) >> 8) |
		((x & 0x0000ff0000000000ULL) >> 24) |
		((x & 0x00ff000000000000ULL) >> 40) |
		((x & 0xff00000000000000ULL) >> 56);
}

inline float _af_byteswap_float32 (float x)
{
	union
	{
		uint32_t i;
		float f;
	} u;
	u.f = x;
	u.i = _af_byteswap_int32(u.i);
	return u.f;
}

inline double _af_byteswap_float64 (double x)
{
	union
	{
		uint64_t i;
		double f;
	} u;
	u.f = x;
	u.i = _af_byteswap_int64(u.i);
	return u.f;
}

inline uint64_t byteswap(uint64_t value) { return _af_byteswap_int64(value); }
inline int64_t byteswap(int64_t value) { return _af_byteswap_int64(value); }
inline uint32_t byteswap(uint32_t value) { return _af_byteswap_int32(value); }
inline int32_t byteswap(int32_t value) { return _af_byteswap_int32(value); }
inline uint16_t byteswap(uint16_t value) { return _af_byteswap_int16(value); }
inline int16_t byteswap(int16_t value) { return _af_byteswap_int16(value); }

inline double byteswap(double value) { return _af_byteswap_float64(value); }
inline float byteswap(float value) { return _af_byteswap_float32(value); }

template <typename T>
T bigToHost(T value)
{
	return _AF_BYTEORDER_NATIVE == AF_BYTEORDER_BIGENDIAN ? value : byteswap(value);
}

template <typename T>
T littleToHost(T value)
{
	return _AF_BYTEORDER_NATIVE == AF_BYTEORDER_LITTLEENDIAN ? value : byteswap(value);
}

template <typename T>
T hostToBig(T value)
{
	return _AF_BYTEORDER_NATIVE == AF_BYTEORDER_BIGENDIAN ? value : byteswap(value);
}

template <typename T>
T hostToLittle(T value)
{
	return _AF_BYTEORDER_NATIVE == AF_BYTEORDER_LITTLEENDIAN ? value : byteswap(value);
}

#endif
