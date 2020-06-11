/*
	Copyright (C) 2011-2013 Michael Pruett. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

	3. The name of the author may not be used to endorse or promote products
	derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
	For each supported file format, test reading and writing audio
	files with all applicable PCM audio data formats.
*/

#include <audiofile.h>
#include <climits>
#include <cmath>
#include <gtest/gtest.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "TestUtilities.h"

struct PCMMapping
{
	double slope, intercept, minClip, maxClip;
};

void getDefaultPCMMapping(int sampleFormat, int bitsPerSample, PCMMapping &m)
{
	switch (sampleFormat)
	{
		case AF_SAMPFMT_TWOSCOMP:
		{
			double s = std::pow(2.0, bitsPerSample - 1);
			m.slope = s;
			m.intercept = 0;
			m.minClip = -s;
			m.maxClip = s - 1;
			break;
		}
		case AF_SAMPFMT_UNSIGNED:
		{
			double s = std::pow(2.0, bitsPerSample - 1);
			m.slope = s;
			m.intercept = s;
			m.minClip = 0;
			m.maxClip = 2 * s - 1;
			break;
		}
		case AF_SAMPFMT_FLOAT:
		case AF_SAMPFMT_DOUBLE:
			m.slope = 1;
			m.intercept = 0;
			m.minClip = 0;
			m.maxClip = 0;
			break;
	}
}

template <typename T, int kSampleFormat, int kBitsPerSample>
void runTestWithChannels(int fileFormat, int channelCount)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("PCMData", &testFileName));

	const int numFrames = 20;
	const int numSamples = numFrames * channelCount;
	T samples[numSamples];
	for (int i=0; i<numFrames; i++)
		for (int c=0; c<channelCount; c++)
			samples[i*channelCount + c] = static_cast<T>(i*i + 3*c + 1);

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitChannels(setup, AF_DEFAULT_TRACK, channelCount);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, kSampleFormat, kBitsPerSample);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file) << "Could not open file for writing";

	afFreeFileSetup(setup);

	PCMMapping mapping, defaultMapping;
	getDefaultPCMMapping(kSampleFormat, kBitsPerSample, defaultMapping);

	afGetPCMMapping(file, AF_DEFAULT_TRACK,
		&mapping.slope, &mapping.intercept, &mapping.minClip, &mapping.maxClip);
	ASSERT_EQ(mapping.slope, defaultMapping.slope);
	ASSERT_EQ(mapping.intercept, defaultMapping.intercept);
	ASSERT_EQ(mapping.minClip, defaultMapping.minClip);
	ASSERT_EQ(mapping.maxClip, defaultMapping.maxClip);

	afGetVirtualPCMMapping(file, AF_DEFAULT_TRACK,
		&mapping.slope, &mapping.intercept, &mapping.minClip, &mapping.maxClip);
	ASSERT_EQ(mapping.slope, defaultMapping.slope);
	ASSERT_EQ(mapping.intercept, defaultMapping.intercept);
	ASSERT_EQ(mapping.minClip, defaultMapping.minClip);
	ASSERT_EQ(mapping.maxClip, defaultMapping.maxClip);

	ASSERT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, samples, numFrames),
		numFrames) <<
		"Number of frames written does not match number of frames requested";

	ASSERT_EQ(afCloseFile(file), 0) << "Error closing file";

	file = afOpenFile(testFileName.c_str(), "r", NULL);
	ASSERT_TRUE(file) << "Could not open file for reading";

	ASSERT_EQ(afGetFileFormat(file, NULL), fileFormat) <<
		"Incorrect file format";

	int sampleFormat, sampleWidth;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	ASSERT_EQ(sampleFormat, kSampleFormat) << "Incorrect sample format";
	ASSERT_EQ(sampleWidth, kBitsPerSample) << "Incorrect sample width";

	ASSERT_EQ(afGetChannels(file, AF_DEFAULT_TRACK), channelCount) <<
		"Incorrect number of channels";

	afGetPCMMapping(file, AF_DEFAULT_TRACK,
		&mapping.slope, &mapping.intercept, &mapping.minClip, &mapping.maxClip);
	ASSERT_EQ(mapping.slope, defaultMapping.slope);
	ASSERT_EQ(mapping.intercept, defaultMapping.intercept);
	ASSERT_EQ(mapping.minClip, defaultMapping.minClip);
	ASSERT_EQ(mapping.maxClip, defaultMapping.maxClip);

	afGetVirtualPCMMapping(file, AF_DEFAULT_TRACK,
		&mapping.slope, &mapping.intercept, &mapping.minClip, &mapping.maxClip);
	ASSERT_EQ(mapping.slope, defaultMapping.slope);
	ASSERT_EQ(mapping.intercept, defaultMapping.intercept);
	ASSERT_EQ(mapping.minClip, defaultMapping.minClip);
	ASSERT_EQ(mapping.maxClip, defaultMapping.maxClip);

	T *samplesRead = new T[numSamples];
	ASSERT_EQ(afReadFrames(file, AF_DEFAULT_TRACK, samplesRead, numFrames),
		numFrames) <<
		"Number of frames read does not match number of frames requested";

	for (int i=0; i<numSamples; i++)
		ASSERT_EQ(samplesRead[i], samples[i]) <<
			"Data read from file does not match data written";

	delete [] samplesRead;

	ASSERT_EQ(afCloseFile(file), 0) << "Error closing file";

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

template <typename T, int kSampleFormat, int kBitsPerSample>
void runTest(int fileFormat)
{
	runTestWithChannels<T, kSampleFormat, kBitsPerSample>(fileFormat, 1);
	runTestWithChannels<T, kSampleFormat, kBitsPerSample>(fileFormat, 2);
	runTestWithChannels<T, kSampleFormat, kBitsPerSample>(fileFormat, 4);
}

void testInt8(int fileFormat)
{
	runTest<int8_t, AF_SAMPFMT_TWOSCOMP, 8>(fileFormat);
}

void testInt8MonoOnly(int fileFormat)
{
	runTestWithChannels<int8_t, AF_SAMPFMT_TWOSCOMP, 8>(fileFormat, 1);
}

void testUInt8(int fileFormat)
{
	runTest<uint8_t, AF_SAMPFMT_UNSIGNED, 8>(fileFormat);
}

void testInt16(int fileFormat)
{
	runTest<int16_t, AF_SAMPFMT_TWOSCOMP, 16>(fileFormat);
}

void testInt16MonoOnly(int fileFormat)
{
	runTestWithChannels<int16_t, AF_SAMPFMT_TWOSCOMP, 16>(fileFormat, 1);
}

void testInt24(int fileFormat)
{
	runTest<int32_t, AF_SAMPFMT_TWOSCOMP, 24>(fileFormat);
}

void testInt32(int fileFormat)
{
	runTest<int32_t, AF_SAMPFMT_TWOSCOMP, 32>(fileFormat);
}

void testFloat32(int fileFormat)
{
	runTest<float, AF_SAMPFMT_FLOAT, 32>(fileFormat);
}

void testFloat64(int fileFormat)
{
	runTest<double, AF_SAMPFMT_DOUBLE, 64>(fileFormat);
}

TEST(AIFF, Int8) { testInt8(AF_FILE_AIFF); }
TEST(AIFF, Int16) { testInt16(AF_FILE_AIFF); }
TEST(AIFF, Int24) { testInt24(AF_FILE_AIFF); }
TEST(AIFF, Int32) { testInt32(AF_FILE_AIFF); }

TEST(AIFFC, Int8) { testInt8(AF_FILE_AIFFC); }
TEST(AIFFC, Int16) { testInt16(AF_FILE_AIFFC); }
TEST(AIFFC, Int24) { testInt24(AF_FILE_AIFFC); }
TEST(AIFFC, Int32) { testInt32(AF_FILE_AIFFC); }
TEST(AIFFC, Float) { testFloat32(AF_FILE_AIFFC); }
TEST(AIFFC, Double) { testFloat64(AF_FILE_AIFFC); }

TEST(WAVE, UInt8) { testUInt8(AF_FILE_WAVE); }
TEST(WAVE, Int16) { testInt16(AF_FILE_WAVE); }
TEST(WAVE, Int24) { testInt24(AF_FILE_WAVE); }
TEST(WAVE, Int32) { testInt32(AF_FILE_WAVE); }
TEST(WAVE, Float) { testFloat32(AF_FILE_WAVE); }
TEST(WAVE, Double) { testFloat64(AF_FILE_WAVE); }

TEST(NeXT, Int8) { testInt8(AF_FILE_NEXTSND); }
TEST(NeXT, Int16) { testInt16(AF_FILE_NEXTSND); }
TEST(NeXT, Int24) { testInt24(AF_FILE_NEXTSND); }
TEST(NeXT, Int32) { testInt32(AF_FILE_NEXTSND); }
TEST(NeXT, Float) { testFloat32(AF_FILE_NEXTSND); }
TEST(NeXT, Double) { testFloat64(AF_FILE_NEXTSND); }

TEST(IRCAM, Int8) { testInt8(AF_FILE_IRCAM); }
TEST(IRCAM, Int16) { testInt16(AF_FILE_IRCAM); }
TEST(IRCAM, Int24) { testInt24(AF_FILE_IRCAM); }
TEST(IRCAM, Int32) { testInt32(AF_FILE_IRCAM); }
TEST(IRCAM, Float) { testFloat32(AF_FILE_IRCAM); }
TEST(IRCAM, Double) { testFloat64(AF_FILE_IRCAM); }

TEST(IFF, Int8) { testInt8MonoOnly(AF_FILE_IFF_8SVX); }

TEST(AVR, Int8) { testInt8MonoOnly(AF_FILE_AVR); }
TEST(AVR, Int16) { testInt16MonoOnly(AF_FILE_AVR); }

TEST(SampleVision, Int16) { testInt16MonoOnly(AF_FILE_SAMPLEVISION); }

TEST(VOC, UInt8) { testUInt8(AF_FILE_VOC); }
TEST(VOC, Int16) { testInt16(AF_FILE_VOC); }

TEST(NIST, Int16) { testInt16(AF_FILE_NIST_SPHERE); }

TEST(CAF, Int8) { testInt8(AF_FILE_CAF); }
TEST(CAF, Int16) { testInt16(AF_FILE_CAF); }
TEST(CAF, Int24) { testInt24(AF_FILE_CAF); }
TEST(CAF, Int32) { testInt32(AF_FILE_CAF); }
TEST(CAF, Float) { testFloat32(AF_FILE_CAF); }
TEST(CAF, Double) { testFloat64(AF_FILE_CAF); }

int main (int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
