/*
	Audio File Library
	Copyright (C) 2013 Michael Pruett <michael@68k.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef Lossless_h
#define Lossless_h

#include <audiofile.h>
#include <gtest/gtest.h>

#include <stdint.h>
#include <unistd.h>

#include "TestUtilities.h"

struct NaturalGenerator
{
	NaturalGenerator() : m_n(0)
	{
	}
	int32_t operator() () { return ++m_n; }

private:
	int32_t m_n;
};

struct LinearCongruentialGenerator
{
	LinearCongruentialGenerator() : m_n(0)
	{
	}
	int32_t operator() ()
	{
		m_n = 1664525 * m_n + 1013904223;
		return static_cast<int32_t>(m_n);
	}

private:
	uint32_t m_n;
};

static inline int32_t trim(int32_t n, int sampleWidth)
{
	unsigned shift = (((sampleWidth + 7) >> 3) << 3) - sampleWidth;
	unsigned maskShift = 32 - sampleWidth;
	n <<= shift;
	return (n << maskShift) >> maskShift;
}

template <typename SampleType, typename Generator>
static void testLossless(const char *prefix, int fileFormat,
	int compressionFormat, int channelCount, int sampleWidth, int frameCount)
{
	SCOPED_TRACE(channelCount);
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile(prefix, &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitChannels(setup, AF_DEFAULT_TRACK, channelCount);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, sampleWidth);
	afInitCompression(setup, AF_DEFAULT_TRACK, compressionFormat);
	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file);
	afFreeFileSetup(setup);

	Generator g;
	SampleType *data = new SampleType[frameCount * channelCount];
	for (int i=0; i<frameCount; i++)
		for (int c=0; c<channelCount; c++)
			data[i*channelCount + c] = trim(g(), sampleWidth) * ((c&1) ? -1 : 1);

	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
	ASSERT_EQ(frameCount, framesWritten);

	ASSERT_EQ(afCloseFile(file), 0);

	file = afOpenFile(testFileName.c_str(), "r", AF_NULL_FILESETUP);
	ASSERT_TRUE(file);
	ASSERT_EQ(compressionFormat, afGetCompression(file, AF_DEFAULT_TRACK));
	int readSampleFormat, readSampleWidth;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &readSampleFormat, &readSampleWidth);
	ASSERT_EQ(AF_SAMPFMT_TWOSCOMP, readSampleFormat);
	ASSERT_EQ(sampleWidth, readSampleWidth);
	ASSERT_EQ(frameCount, afGetFrameCount(file, AF_DEFAULT_TRACK));

	SampleType *readData = new SampleType[frameCount * channelCount];
	AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
	ASSERT_EQ(frameCount, framesRead);

	for (int i=0; i<frameCount; i++)
		for (int c=0; c<channelCount; c++)
			EXPECT_EQ(data[i*channelCount + c], readData[i*channelCount + c]) <<
				"failed at " << i;

	// Read entire file with a seek before each read operation.
	for (AFframecount offset = 0; offset < frameCount; offset += 373)
	{
		ASSERT_EQ(offset, afSeekFrame(file, AF_DEFAULT_TRACK, offset));

		AFframecount framesToRead = 1091;
		framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, framesToRead);
		ASSERT_EQ(std::min(framesToRead, frameCount - offset), framesRead);

		for (int i=0; i<framesRead; i++)
			for (int c=0; c<channelCount; c++)
				EXPECT_EQ(data[(i+offset)*channelCount + c],
					readData[i*channelCount + c]) << "failed at " << i;
	}

	// Read entire file sequentially in multiple read operations.
	ASSERT_EQ(afSeekFrame(file, AF_DEFAULT_TRACK, 0), 0);

	AFframecount framesToRead = 1087;
	for (AFframecount offset = 0; offset < frameCount; offset += framesToRead)
	{
		framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, framesToRead);
		ASSERT_EQ(std::min(framesToRead, frameCount - offset), framesRead);

		for (int i=0; i<framesRead; i++)
			for (int c=0; c<channelCount; c++)
				EXPECT_EQ(data[(i+offset)*channelCount + c],
					readData[i*channelCount + c]) << "failed at " << i;
	}

	ASSERT_EQ(0, afCloseFile(file));

	delete [] data;
	delete [] readData;

	ASSERT_EQ(0, ::unlink(testFileName.c_str()));
}

template <typename SampleType>
static void testLossless(const char *prefix, int fileFormat,
	int compressionFormat, int channelCount, int sampleWidth, int frameCount)
{
	{
		SCOPED_TRACE("Sequential");
		testLossless<SampleType, NaturalGenerator>(prefix, fileFormat, compressionFormat, channelCount, sampleWidth, frameCount);
	}
	{
		SCOPED_TRACE("Pseudo-random");
		testLossless<SampleType, LinearCongruentialGenerator>(prefix, fileFormat, compressionFormat, channelCount, sampleWidth, frameCount);
	}
}

#endif
