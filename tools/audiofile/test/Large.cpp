/*
	Copyright (C) 2010-2011, Michael Pruett. All rights reserved.

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
	This program tests writing and reading audio files with more
	than 2^24 frames.

	This program serves as a regression test for a bug in which
	writing certain audio file formats with more than 2^24 frames
	would produce files with incorrect sizes.
*/

#include <audiofile.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "TestUtilities.h"

void testLargeFile(int fileFormat)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("ChannelMatrix", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	afFreeFileSetup(setup);
	ASSERT_TRUE(file) << "Could not open file for writing";

	const int bufferFrameCount = 32768;
	int16_t *data = new int16_t[bufferFrameCount];
	const AFframecount frameCount = 0x1000007;
	AFframecount framesWritten = 0;
	while (framesWritten < frameCount)
	{
		AFframecount framesToWrite = bufferFrameCount;
		if (framesToWrite > frameCount - framesWritten)
			framesToWrite = frameCount - framesWritten;
		int dataValue = framesWritten % 32749;
		for (int i=0; i<bufferFrameCount; i++, dataValue++)
			data[i] = dataValue % 32749;
		ASSERT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, data, framesToWrite),
			framesToWrite);
		framesWritten += framesToWrite;
	}
	ASSERT_EQ(afGetFrameCount(file, AF_DEFAULT_TRACK), frameCount) <<
		"Incorrect frame count for file";
	afCloseFile(file);

	file = afOpenFile(testFileName.c_str(), "r", AF_NULL_FILESETUP);
	ASSERT_TRUE(file) << "Could not open file for reading";
	ASSERT_EQ(afGetFrameCount(file, AF_DEFAULT_TRACK), frameCount) <<
		"Incorrect frame count for file opened for reading";
	ASSERT_EQ(afGetTrackBytes(file, AF_DEFAULT_TRACK),
		frameCount * sizeof (int16_t)) <<
		"Incorrect byte size in file opened for reading";

	AFframecount framesRead = 0;
	while (framesRead < frameCount)
	{
		AFframecount framesToRead = bufferFrameCount;
		if (framesToRead > frameCount - framesRead)
			framesToRead = frameCount - framesRead;
		afReadFrames(file, AF_DEFAULT_TRACK, data, framesToRead);
		bool valid = true;
		int dataValue = framesRead % 32749;
		for (int i=0; i<framesToRead; i++, dataValue++)
			if (data[i] != dataValue % 32749)
			{
				valid = false;
				break;
			}
		ASSERT_TRUE(valid) << "Data read does not match data written";
		framesRead += framesToRead;
	}
	delete [] data;
	afCloseFile(file);
	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(Large, AIFF)
{
	testLargeFile(AF_FILE_AIFF);
}

TEST(Large, AIFFC)
{
	testLargeFile(AF_FILE_AIFFC);
}

TEST(Large, NeXT)
{
	testLargeFile(AF_FILE_NEXTSND);
}

TEST(Large, WAVE)
{
	testLargeFile(AF_FILE_WAVE);
}

TEST(Large, IRCAM)
{
	testLargeFile(AF_FILE_IRCAM);
}

TEST(Large, AVR)
{
	testLargeFile(AF_FILE_AVR);
}

TEST(Large, CAF)
{
	testLargeFile(AF_FILE_CAF);
}

int main (int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
