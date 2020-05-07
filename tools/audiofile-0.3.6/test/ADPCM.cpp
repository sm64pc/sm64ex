/*
	Audio File Library

	Copyright (C) 2012, Michael Pruett <michael@68k.org>

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
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>

#include "TestUtilities.h"

static const int kIMABytesPerPacketQT = 34;
static const int kIMABytesPerPacketWAVE = 256;

static const int kIMAFramesPerPacketQT = 64;
static const int kIMAFramesPerPacketWAVE = 505;

static const int kIMAThresholdQT = 128;
static const int kIMAThresholdWAVE = 16;

static const int kMSADPCMBytesPerPacket = 256;
static const int kMSADPCMFramesPerPacket = 500;
static const int kMSADPCMThreshold = 16;

static void testADPCM(int fileFormat, int compressionFormat, int channelCount,
	int bytesPerPacket, int framesPerPacket, int frameCount, int threshold)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("ADPCM", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitChannels(setup, AF_DEFAULT_TRACK, channelCount);
	afInitCompression(setup, AF_DEFAULT_TRACK, compressionFormat);
	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file);
	afFreeFileSetup(setup);

	int16_t *data = new int16_t[frameCount * channelCount];
	for (int i=0; i<frameCount; i++)
		for (int c=0; c<channelCount; c++)
			data[i*channelCount + c] = i * ((c&1) ? -1 : 1);
	
	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
	ASSERT_EQ(framesWritten, frameCount);

	ASSERT_EQ(afCloseFile(file), 0);

	file = afOpenFile(testFileName.c_str(), "r", AF_NULL_FILESETUP);
	ASSERT_TRUE(file);
	ASSERT_EQ(afGetCompression(file, AF_DEFAULT_TRACK), compressionFormat);
	ASSERT_EQ(afGetFrameCount(file, AF_DEFAULT_TRACK), frameCount);
	ASSERT_EQ(afGetTrackBytes(file, AF_DEFAULT_TRACK),
		(bytesPerPacket * frameCount) / framesPerPacket);

	int16_t *readData = new int16_t[frameCount * channelCount];
	AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
	ASSERT_EQ(framesRead, frameCount);

	for (int i=0; i<frameCount; i++)
		for (int c=0; c<channelCount; c++)
			EXPECT_LE(std::abs(data[i*channelCount + c] - readData[i*channelCount + c]), threshold);

	int16_t *offsetReadData = new int16_t[frameCount * channelCount];

	// Read entire file with a seek before each read operation.
	for (AFframecount offset = 0; offset < frameCount; offset += framesPerPacket + 3)
	{
		ASSERT_EQ(afSeekFrame(file, AF_DEFAULT_TRACK, offset), offset);

		AFframecount framesToRead = 1091;
		framesRead = afReadFrames(file, AF_DEFAULT_TRACK, offsetReadData, framesToRead);
		ASSERT_EQ(framesRead, std::min(framesToRead, frameCount - offset));

		for (int i=0; i<framesRead; i++)
			for (int c=0; c<channelCount; c++)
				EXPECT_EQ(readData[(i+offset)*channelCount + c],
					offsetReadData[i*channelCount + c]);
	}

	// Read entire file sequentially in multiple read operations.
	ASSERT_EQ(afSeekFrame(file, AF_DEFAULT_TRACK, 0), 0);

	AFframecount framesToRead = 1087;
	for (AFframecount offset = 0; offset < frameCount; offset += framesToRead)
	{
		framesRead = afReadFrames(file, AF_DEFAULT_TRACK, offsetReadData, framesToRead);
		ASSERT_EQ(framesRead, std::min(framesToRead, frameCount - offset));

		for (int i=0; i<framesRead; i++)
			for (int c=0; c<channelCount; c++)
				EXPECT_EQ(readData[(i+offset)*channelCount + c],
					offsetReadData[i*channelCount + c]);
	}

	ASSERT_EQ(afCloseFile(file), 0);

	delete [] data;
	delete [] readData;
	delete [] offsetReadData;

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(IMA, CAF)
{
	for (int channelCount=1; channelCount<=2; channelCount++)
		testADPCM(AF_FILE_CAF, AF_COMPRESSION_IMA, channelCount,
			channelCount * kIMABytesPerPacketQT, kIMAFramesPerPacketQT,
			500 * kIMAFramesPerPacketQT, kIMAThresholdQT);
}

TEST(IMA, AIFFC)
{
	for (int channelCount=1; channelCount<=2; channelCount++)
		testADPCM(AF_FILE_AIFFC, AF_COMPRESSION_IMA, channelCount,
			channelCount * kIMABytesPerPacketQT, kIMAFramesPerPacketQT,
			500 * kIMAFramesPerPacketQT, kIMAThresholdQT);
}

TEST(IMA, WAVE)
{
	for (int channelCount=1; channelCount<=2; channelCount++)
		testADPCM(AF_FILE_WAVE, AF_COMPRESSION_IMA, channelCount,
			channelCount * kIMABytesPerPacketWAVE, kIMAFramesPerPacketWAVE,
			50 * kIMAFramesPerPacketWAVE, kIMAThresholdWAVE);
}

TEST(MSADPCM, WAVE)
{
	for (int channelCount=1; channelCount<=2; channelCount++)
		testADPCM(AF_FILE_WAVE, AF_COMPRESSION_MS_ADPCM, channelCount,
			channelCount * kMSADPCMBytesPerPacket, kMSADPCMFramesPerPacket,
			50 * kMSADPCMFramesPerPacket, kMSADPCMThreshold);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
