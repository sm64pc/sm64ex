/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>
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
	afinternal.h

	This file defines the internal structures for the Audio File Library.
*/

#ifndef AFINTERNAL_H
#define AFINTERNAL_H

#include <sys/types.h>
#include "audiofile.h"
#include "error.h"

enum status
{
	AF_SUCCEED = 0,
	AF_FAIL = -1
};

union AFPVu
{
	long	l;
	double	d;
	void	*v;
};

struct InstParamInfo
{
	int id;
	int type;
	const char *name;
	AFPVu defaultValue;
};

struct Loop
{
	int	id;
	int	mode;	/* AF_LOOP_MODE_... */
	int	count;	/* how many times the loop is played */
	int	beginMarker, endMarker;
	int	trackid;
};

struct LoopSetup
{
	int	id;
};

struct Miscellaneous
{
	int id;
	int type;
	int size;

	void *buffer;

	int position;	// offset within the miscellaneous chunk
};

struct MiscellaneousSetup
{
	int	id;
	int	type;
	int	size;
};

struct TrackSetup;

class File;
struct Track;

enum
{
	_AF_VALID_FILEHANDLE = 38212,
	_AF_VALID_FILESETUP = 38213
};

enum
{
	_AF_READ_ACCESS = 1,
	_AF_WRITE_ACCESS = 2
};

// The following are tokens for compression parameters in PV lists.
enum
{
	_AF_MS_ADPCM_NUM_COEFFICIENTS = 800,	/* type: long */
	_AF_MS_ADPCM_COEFFICIENTS = 801,		/* type: array of int16_t[2] */
	_AF_IMA_ADPCM_TYPE = 810,
	_AF_IMA_ADPCM_TYPE_WAVE = 1,
	_AF_IMA_ADPCM_TYPE_QT = 2,
	_AF_CODEC_DATA = 900,		// type: pointer
	_AF_CODEC_DATA_SIZE = 901	// type: long
};

/* NeXT/Sun sampling rate */
#define _AF_SRATE_CODEC (8012.8210513)

#endif
