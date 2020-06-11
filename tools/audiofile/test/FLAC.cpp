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

#include <audiofile.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <unistd.h>

#include "Lossless.h"
#include "TestUtilities.h"

template <typename SampleType>
static void testFLAC(int channelCount, int sampleWidth, int frameCount)
{
	testLossless<SampleType>("FLAC", AF_FILE_FLAC, AF_COMPRESSION_FLAC,
		channelCount, sampleWidth, frameCount);
}

TEST(FLAC, FLAC_16)
{
	for (int channelCount=1; channelCount<=8; channelCount++)
		testFLAC<int16_t>(channelCount, 16, 82421);
}

TEST(FLAC, FLAC_24)
{
	for (int channelCount=1; channelCount<=8; channelCount++)
		testFLAC<int32_t>(channelCount, 24, 82421);
}

static void testInvalidSampleFormat(int sampleFormat, int sampleWidth)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("FLAC", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_FLAC);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, sampleFormat, sampleWidth);
	afInitCompression(setup, AF_DEFAULT_TRACK, AF_COMPRESSION_FLAC);
	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_FALSE(file);
	afFreeFileSetup(setup);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(FLAC, InvalidSampleWidths)
{
	IgnoreErrors ignoreErrors;

	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
	{
		if (sampleWidth == 16 || sampleWidth == 24)
			continue;

		testInvalidSampleFormat(AF_SAMPFMT_TWOSCOMP, sampleWidth);
	}
}

TEST(FLAC, InvalidSampleFormats)
{
	IgnoreErrors ignoreErrors;

	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
		testInvalidSampleFormat(AF_SAMPFMT_UNSIGNED, sampleWidth);

	testInvalidSampleFormat(AF_SAMPFMT_FLOAT, 32);
	testInvalidSampleFormat(AF_SAMPFMT_DOUBLE, 64);
}

TEST(FLAC, InvalidChannels)
{
	IgnoreErrors ignoreErrors;

	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("FLAC", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_FLAC);
	afInitChannels(setup, AF_DEFAULT_TRACK, 9);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
	afInitCompression(setup, AF_DEFAULT_TRACK, AF_COMPRESSION_FLAC);
	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_FALSE(file);
	afFreeFileSetup(setup);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
