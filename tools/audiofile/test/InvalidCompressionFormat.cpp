/*
	Audio File Library
	Copyright (C) 2012-2013, Michael Pruett <michael@68k.org>

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

#include "config.h"

#include <audiofile.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "TestUtilities.h"

static const int kNativeByteOrder =
#ifdef WORDS_BIGENDIAN
	AF_BYTEORDER_BIGENDIAN;
#else
	AF_BYTEORDER_LITTLEENDIAN;
#endif

static const int kNonNativeByteOrder =
#if WORDS_BIGENDIAN
	AF_BYTEORDER_LITTLEENDIAN;
#else
	AF_BYTEORDER_BIGENDIAN;
#endif

static void runTest(int fileFormat, int compressionFormat, int channelCount = 1,
	int sampleFormat = AF_SAMPFMT_TWOSCOMP, int sampleWidth = 16,
	int byteOrder = kNativeByteOrder)
{
	IgnoreErrors ignoreErrors;

	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("InvalidCompressionFormat", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, sampleFormat, sampleWidth);
	afInitChannels(setup, AF_DEFAULT_TRACK, channelCount);
	afInitCompression(setup, AF_DEFAULT_TRACK, compressionFormat);
	afInitByteOrder(setup, AF_DEFAULT_TRACK, byteOrder);
	ASSERT_TRUE(afOpenFile(testFileName.c_str(), "w", setup) == AF_NULL_FILEHANDLE);
	afFreeFileSetup(setup);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(AIFF, mulaw) { runTest(AF_FILE_AIFF, AF_COMPRESSION_G711_ULAW); }
TEST(AIFF, Alaw) { runTest(AF_FILE_AIFF, AF_COMPRESSION_G711_ALAW); }
TEST(AIFF, IMA) { runTest(AF_FILE_AIFF, AF_COMPRESSION_IMA); }
TEST(AIFF, MSADPCM) { runTest(AF_FILE_AIFF, AF_COMPRESSION_MS_ADPCM); }

TEST(AIFFC, MSADPCM) { runTest(AF_FILE_AIFFC, AF_COMPRESSION_MS_ADPCM); }

TEST(NeXT, IMA) { runTest(AF_FILE_NEXTSND, AF_COMPRESSION_IMA); }
TEST(NeXT, MSADPCM) { runTest(AF_FILE_NEXTSND, AF_COMPRESSION_MS_ADPCM); }

TEST(IRCAM, IMA) { runTest(AF_FILE_BICSF, AF_COMPRESSION_IMA); }
TEST(IRCAM, MSADPCM) { runTest(AF_FILE_BICSF, AF_COMPRESSION_MS_ADPCM); }

TEST(AVR, mulaw) { runTest(AF_FILE_AVR, AF_COMPRESSION_G711_ULAW); }
TEST(AVR, Alaw) { runTest(AF_FILE_AVR, AF_COMPRESSION_G711_ALAW); }
TEST(AVR, IMA) { runTest(AF_FILE_AVR, AF_COMPRESSION_IMA); }
TEST(AVR, MSADPCM) { runTest(AF_FILE_AVR, AF_COMPRESSION_MS_ADPCM); }

TEST(IFF, mulaw) { runTest(AF_FILE_IFF_8SVX, AF_COMPRESSION_G711_ULAW); }
TEST(IFF, Alaw) { runTest(AF_FILE_IFF_8SVX, AF_COMPRESSION_G711_ALAW); }
TEST(IFF, IMA) { runTest(AF_FILE_IFF_8SVX, AF_COMPRESSION_IMA); }
TEST(IFF, MSADPCM) { runTest(AF_FILE_IFF_8SVX, AF_COMPRESSION_MS_ADPCM); }

TEST(SampleVision, mulaw) { runTest(AF_FILE_SAMPLEVISION, AF_COMPRESSION_G711_ULAW); }
TEST(SampleVision, Alaw) { runTest(AF_FILE_SAMPLEVISION, AF_COMPRESSION_G711_ALAW); }
TEST(SampleVision, IMA) { runTest(AF_FILE_SAMPLEVISION, AF_COMPRESSION_IMA); }
TEST(SampleVision, MSADPCM) { runTest(AF_FILE_SAMPLEVISION, AF_COMPRESSION_MS_ADPCM); }

TEST(VOC, IMA) { runTest(AF_FILE_VOC, AF_COMPRESSION_IMA); }
TEST(VOC, MSADPCM) { runTest(AF_FILE_VOC, AF_COMPRESSION_MS_ADPCM); }

TEST(NIST, IMA) { runTest(AF_FILE_NIST_SPHERE, AF_COMPRESSION_IMA); }
TEST(NIST, MSADPCM) { runTest(AF_FILE_NIST_SPHERE, AF_COMPRESSION_MS_ADPCM); }

TEST(CAF, MSADPCM) { runTest(AF_FILE_CAF, AF_COMPRESSION_MS_ADPCM); }

/*
	Test that opening an audio file with an invalid number of channels
	results in failure.
*/

TEST(AIFFC, IMA) { runTest(AF_FILE_AIFFC, AF_COMPRESSION_IMA, 3); }

TEST(WAVE, IMA) { runTest(AF_FILE_WAVE, AF_COMPRESSION_IMA, 3); }
TEST(WAVE, MSADPCM) { runTest(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, 3); }

TEST(CAF, IMA) { runTest(AF_FILE_CAF, AF_COMPRESSION_IMA, 3); }

/*
	Test that opening an audio file with an invalid sample format results
	in failure.
*/

TEST(Mulaw, Signed)
{
	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
	{
		if (sampleWidth==16)
			continue;
		runTest(AF_FILE_AIFFC, AF_COMPRESSION_G711_ULAW, 1, AF_SAMPFMT_TWOSCOMP, sampleWidth);
	}
}

TEST(Mulaw, Unsigned)
{
	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
		runTest(AF_FILE_AIFFC, AF_COMPRESSION_G711_ULAW, 1, AF_SAMPFMT_UNSIGNED, sampleWidth);
}

TEST(Mulaw, Float)
{
	runTest(AF_FILE_AIFFC, AF_COMPRESSION_G711_ULAW, 1, AF_SAMPFMT_FLOAT, 32);
}

TEST(Mulaw, Double)
{
	runTest(AF_FILE_AIFFC, AF_COMPRESSION_G711_ULAW, 1, AF_SAMPFMT_DOUBLE, 64);
}

TEST(IMA, Signed)
{
	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
	{
		if (sampleWidth==16)
			continue;
		runTest(AF_FILE_AIFFC, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_TWOSCOMP, sampleWidth);
		runTest(AF_FILE_WAVE, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_TWOSCOMP, sampleWidth);
		runTest(AF_FILE_CAF, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_TWOSCOMP, sampleWidth);
	}
}

TEST(IMA, Unsigned)
{
	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
	{
		runTest(AF_FILE_AIFFC, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_UNSIGNED, sampleWidth);
		runTest(AF_FILE_WAVE, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_UNSIGNED, sampleWidth);
		runTest(AF_FILE_CAF, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_UNSIGNED, sampleWidth);
	}
}

TEST(IMA, Float)
{
	runTest(AF_FILE_AIFFC, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_FLOAT, 32);
	runTest(AF_FILE_WAVE, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_FLOAT, 32);
	runTest(AF_FILE_CAF, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_FLOAT, 32);
}

TEST(IMA, Double)
{
	runTest(AF_FILE_AIFFC, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_DOUBLE, 64);
	runTest(AF_FILE_WAVE, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_DOUBLE, 64);
	runTest(AF_FILE_CAF, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_DOUBLE, 64);
}

TEST(MSADPCM, Signed)
{
	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
		if (sampleWidth!=16)
			runTest(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, 1, AF_SAMPFMT_TWOSCOMP, sampleWidth);
}

TEST(MSADPCM, Unsigned)
{
	for (int sampleWidth=1; sampleWidth<=32; sampleWidth++)
		runTest(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, 1, AF_SAMPFMT_UNSIGNED, sampleWidth);
}

TEST(MSADPCM, Float)
{
	runTest(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, 1, AF_SAMPFMT_FLOAT, 32);
}

TEST(MSADPCM, Double)
{
	runTest(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, 1, AF_SAMPFMT_DOUBLE, 64);
}

TEST(Mulaw, InvalidByteOrder)
{
	runTest(AF_FILE_WAVE, AF_COMPRESSION_G711_ULAW, 1, AF_SAMPFMT_TWOSCOMP, 16,
		kNonNativeByteOrder);
}

TEST(Alaw, InvalidByteOrder)
{
	runTest(AF_FILE_WAVE, AF_COMPRESSION_G711_ALAW, 1, AF_SAMPFMT_TWOSCOMP, 16,
		kNonNativeByteOrder);
}

TEST(IMA, InvalidByteOrder)
{
	runTest(AF_FILE_WAVE, AF_COMPRESSION_IMA, 1, AF_SAMPFMT_TWOSCOMP, 16,
		kNonNativeByteOrder);
}

TEST(MSADPCM, InvalidByteOrder)
{
	runTest(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, 1, AF_SAMPFMT_TWOSCOMP, 16,
		kNonNativeByteOrder);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
