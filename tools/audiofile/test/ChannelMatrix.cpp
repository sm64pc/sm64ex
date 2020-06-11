/*
	Copyright (C) 2011, Michael Pruett. All rights reserved.

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

#include <audiofile.h>
#include <gtest/gtest.h>
#include <string>

#include "TestUtilities.h"

template <typename T>
void testChannelMatrixReading(int sampleFormat, int sampleWidth)
{
	// Create test file.
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("ChannelMatrix", &testFileName));

	const int channelCount = 2;
	const int frameCount = 10;
	const T samples[channelCount * frameCount] =
	{
		2, 3, 5, 7, 11,
		13, 17, 19, 23, 29,
		31, 37, 41, 43, 47,
		53, 59, 61, 67, 71
	};
	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_AIFFC);
	afInitChannels(setup, AF_DEFAULT_TRACK, 2);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, sampleFormat, sampleWidth);
	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	afFreeFileSetup(setup);
	EXPECT_TRUE(file);

	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK,
		samples, frameCount);
	EXPECT_EQ(framesWritten, frameCount);

	EXPECT_EQ(afCloseFile(file), 0);

	// Open file for reading and read data using different channel matrices.
	file = afOpenFile(testFileName.c_str(), "r", NULL);
	EXPECT_TRUE(file);

	EXPECT_EQ(afGetChannels(file, AF_DEFAULT_TRACK), 2);
	EXPECT_EQ(afGetFrameCount(file, AF_DEFAULT_TRACK), frameCount);

	afSetVirtualChannels(file, AF_DEFAULT_TRACK, 1);

	for (int c=0; c<2; c++)
	{
		double channelMatrix[2] = { 0, 0 };
		channelMatrix[c] = 1;
		afSetChannelMatrix(file, AF_DEFAULT_TRACK, channelMatrix);

		EXPECT_EQ(afSeekFrame(file, AF_DEFAULT_TRACK, 0), 0);

		T *readSamples = new T[frameCount]; 
		AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK,
			readSamples, frameCount);
		EXPECT_EQ(framesRead, frameCount);

		for (int i=0; i<frameCount; i++)
			EXPECT_EQ(readSamples[i], samples[2*i + c]);

		delete [] readSamples;
	}

	EXPECT_EQ(afCloseFile(file), 0);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(ChannelMatrix, ReadInt8)
{
	testChannelMatrixReading<int8_t>(AF_SAMPFMT_TWOSCOMP, 8);
}

TEST(ChannelMatrix, ReadInt16)
{
	testChannelMatrixReading<int16_t>(AF_SAMPFMT_TWOSCOMP, 16);
}

TEST(ChannelMatrix, ReadInt24)
{
	testChannelMatrixReading<int32_t>(AF_SAMPFMT_TWOSCOMP, 24);
}

TEST(ChannelMatrix, ReadInt32)
{
	testChannelMatrixReading<int32_t>(AF_SAMPFMT_TWOSCOMP, 32);
}

TEST(ChannelMatrix, ReadFloat)
{
	testChannelMatrixReading<float>(AF_SAMPFMT_FLOAT, 32);
}

TEST(ChannelMatrix, ReadDouble)
{
	testChannelMatrixReading<double>(AF_SAMPFMT_DOUBLE, 64);
}

template <typename T>
void testChannelMatrixWriting(int sampleFormat, int sampleWidth)
{
	const int channelCount = 2;
	const int frameCount = 10;
	const T samples[channelCount * frameCount] =
	{
		2, 3, 5, 7, 11,
		13, 17, 19, 23, 29,
		31, 37, 41, 43, 47,
		53, 59, 61, 67, 71
	};

	for (int c=0; c<2; c++)
	{
		// Create test file.
		std::string testFileName;
		ASSERT_TRUE(createTemporaryFile("ChannelMatrix", &testFileName));

		AFfilesetup setup = afNewFileSetup();
		afInitFileFormat(setup, AF_FILE_AIFFC);
		afInitChannels(setup, AF_DEFAULT_TRACK, 1);
		afInitSampleFormat(setup, AF_DEFAULT_TRACK, sampleFormat, sampleWidth);
		AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
		afFreeFileSetup(setup);
		EXPECT_TRUE(file);

		afSetVirtualChannels(file, AF_DEFAULT_TRACK, 2);

		double channelMatrix[2] = { 0, 0 };
		channelMatrix[c] = 1;
		afSetChannelMatrix(file, AF_DEFAULT_TRACK, channelMatrix);

		AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK,
			samples, frameCount);
		EXPECT_EQ(framesWritten, frameCount);

		EXPECT_EQ(afCloseFile(file), 0);

		// Open file for reading.
		file = afOpenFile(testFileName.c_str(), "r", NULL);
		EXPECT_TRUE(file);

		EXPECT_EQ(afGetChannels(file, AF_DEFAULT_TRACK), 1);
		EXPECT_EQ(afGetFrameCount(file, AF_DEFAULT_TRACK), frameCount);

		T *readSamples = new T[frameCount]; 
		AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK,
			readSamples, frameCount);
		EXPECT_EQ(framesRead, frameCount);

		for (int i=0; i<frameCount; i++)
			EXPECT_EQ(readSamples[i], samples[2*i + c]);

		delete [] readSamples;

		EXPECT_EQ(afCloseFile(file), 0);

		ASSERT_EQ(::unlink(testFileName.c_str()), 0);
	}
}

TEST(ChannelMatrix, WriteInt8)
{
	testChannelMatrixWriting<int8_t>(AF_SAMPFMT_TWOSCOMP, 8);
}

TEST(ChannelMatrix, WriteInt16)
{
	testChannelMatrixWriting<int16_t>(AF_SAMPFMT_TWOSCOMP, 16);
}

TEST(ChannelMatrix, WriteInt24)
{
	testChannelMatrixWriting<int32_t>(AF_SAMPFMT_TWOSCOMP, 24);
}

TEST(ChannelMatrix, WriteInt32)
{
	testChannelMatrixWriting<int32_t>(AF_SAMPFMT_TWOSCOMP, 32);
}

TEST(ChannelMatrix, WriteFloat)
{
	testChannelMatrixWriting<float>(AF_SAMPFMT_FLOAT, 32);
}

TEST(ChannelMatrix, WriteDouble)
{
	testChannelMatrixWriting<double>(AF_SAMPFMT_DOUBLE, 64);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
