/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.

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

/*
	pcm.h

	This file defines various constants for PCM mapping.
*/

#ifndef PCM_H
#define PCM_H

/*
	SLOPE_INTN = 2^(N-1)
*/
#define SLOPE_INT8 (128.0)
#define SLOPE_INT16 (32768.0)
#define SLOPE_INT24 (8388608.0)
#define SLOPE_INT32 (2147483648.0)

/*
	INTERCEPT_U_INTN = 2^(N-1)
*/
#define INTERCEPT_U_INT8 (128.0)
#define INTERCEPT_U_INT16 (32768.0)
#define INTERCEPT_U_INT24 (8388608.0)
#define INTERCEPT_U_INT32 (2147483648.0)

/*
	MIN_INTN = -(2^(N-1))
*/
#define MIN_INT8 (-128.0)
#define MIN_INT16 (-32768.0)
#define MIN_INT24 (-8388608.0)
#define MIN_INT32 (-2147483648.0)

/*
	MAX_INTN = 2^(N-1) - 1
*/
#define MAX_INT8 127.0
#define MAX_INT16 32767.0
#define MAX_INT24 8388607.0
#define MAX_INT32 2147483647.0

/*
	MAX_U_INTN = 2^N - 1
*/
#define MAX_U_INT8 255.0
#define MAX_U_INT16 65535.0
#define MAX_U_INT24 16777215.0
#define MAX_U_INT32 4294967295.0

extern const PCMInfo _af_default_signed_integer_pcm_mappings[];
extern const PCMInfo _af_default_unsigned_integer_pcm_mappings[];
extern const PCMInfo _af_default_float_pcm_mapping;
extern const PCMInfo _af_default_double_pcm_mapping;

#endif
