/*
	Audio File Library
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
#include "AudioFormat.h"

#include "afinternal.h"
#include "byteorder.h"
#include "compression.h"
#include "units.h"
#include "util.h"
#include <assert.h>
#include <stdio.h>

size_t AudioFormat::bytesPerSample(bool stretch3to4) const
{
	switch (sampleFormat)
	{
		case AF_SAMPFMT_FLOAT:
			return sizeof (float);
		case AF_SAMPFMT_DOUBLE:
			return sizeof (double);
		default:
		{
			int size = (sampleWidth + 7) / 8;
			if (compressionType == AF_COMPRESSION_NONE &&
				size == 3 && stretch3to4)
				size = 4;
			return size;
		}
	}
}

size_t AudioFormat::bytesPerFrame(bool stretch3to4) const
{
	return bytesPerSample(stretch3to4) * channelCount;
}

size_t AudioFormat::bytesPerSample() const
{
	return bytesPerSample(!isPacked());
}

size_t AudioFormat::bytesPerFrame() const
{
	return bytesPerFrame(!isPacked());
}

bool AudioFormat::isInteger() const
{
	return sampleFormat == AF_SAMPFMT_TWOSCOMP ||
		sampleFormat == AF_SAMPFMT_UNSIGNED;
}

bool AudioFormat::isSigned() const
{
	return sampleFormat == AF_SAMPFMT_TWOSCOMP;
}

bool AudioFormat::isUnsigned() const
{
	return sampleFormat == AF_SAMPFMT_UNSIGNED;
}

bool AudioFormat::isFloat() const
{
	return sampleFormat == AF_SAMPFMT_FLOAT ||
		sampleFormat == AF_SAMPFMT_DOUBLE;
}

bool AudioFormat::isCompressed() const
{
	return compressionType != AF_COMPRESSION_NONE;
}

bool AudioFormat::isUncompressed() const
{
	return compressionType == AF_COMPRESSION_NONE;
}

void AudioFormat::computeBytesPerPacketPCM()
{
	assert(isUncompressed());
	int bytesPerSample = (sampleWidth + 7) / 8;
	bytesPerPacket = bytesPerSample * channelCount;
}

std::string AudioFormat::description() const
{
	std::string d;
	char s[1024];
	/* sampleRate, channelCount */
	sprintf(s, "{ %7.2f Hz %d ch ", sampleRate, channelCount);
	d += s;

	/* sampleFormat, sampleWidth */
	switch (sampleFormat)
	{
		case AF_SAMPFMT_TWOSCOMP:
			sprintf(s, "%db 2 ", sampleWidth);
			break;
		case AF_SAMPFMT_UNSIGNED:
			sprintf(s, "%db u ", sampleWidth);
			break;
		case AF_SAMPFMT_FLOAT:
			sprintf(s, "flt ");
			break;
		case AF_SAMPFMT_DOUBLE:
			sprintf(s, "dbl ");
			break;
		default:
			assert(false);
			break;
	}

	d += s;

	/* pcm */
	sprintf(s, "(%.30g+-%.30g [%.30g,%.30g]) ",
		pcm.intercept, pcm.slope,
		pcm.minClip, pcm.maxClip);
	d += s;

	/* byteOrder */
	switch (byteOrder)
	{
		case AF_BYTEORDER_BIGENDIAN:
			d += "big ";
			break;
		case AF_BYTEORDER_LITTLEENDIAN:
			d += "little ";
			break;
		default:
			assert(false);
			break;
	}

	if (isCompressed())
	{
		const CompressionUnit *unit = _af_compression_unit_from_id(compressionType);
		assert(unit);
		d += "compression: ";
		d += unit->label;
	}

	return d;
}
