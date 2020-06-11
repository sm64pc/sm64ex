/*
	Copyright (C) 2016, Michael Pruett. All rights reserved.

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
	Validate conversion when changing both sample format and channel count.
*/

#include <gtest/gtest.h>
#include <audiofile.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <climits>
#include <limits>

#include "TestUtilities.h"

class FormatWithChannels : public testing::Test
{
protected:
	virtual void SetUp()
	{
		ASSERT_TRUE(createTemporaryFile("FormatWithChannels", &m_testFileName));
	}
	virtual void TearDown()
	{
		ASSERT_EQ(::unlink(m_testFileName.c_str()), 0);
	}

	AFfilehandle createTestFile(int sampleWidth, int channelCount)
	{
		AFfilesetup setup = afNewFileSetup();
		afInitFileFormat(setup, AF_FILE_AIFFC);
		afInitChannels(setup, AF_DEFAULT_TRACK, channelCount);
		afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, sampleWidth);
		AFfilehandle file = afOpenFile(m_testFileName.c_str(), "w", setup);
		afFreeFileSetup(setup);
		return file;
	}
	AFfilehandle openTestFile(int sampleWidth, int channelCount)
	{
		AFfilehandle file = afOpenFile(m_testFileName.c_str(), "r", AF_NULL_FILESETUP);
		afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, sampleWidth);
		afSetVirtualChannels(file, AF_DEFAULT_TRACK, channelCount);
		int fileChannels = afGetChannels(file, AF_DEFAULT_TRACK);

		double channelMatrix[fileChannels * channelCount];
		for (int i = 0; i < fileChannels; i++)
			for (int j = 0; j < channelCount; j++)
				channelMatrix[j * fileChannels + i] = (i == j) ? 1 : 0;
		afSetChannelMatrix(file, AF_DEFAULT_TRACK, channelMatrix);
		return file;
	}

private:
	std::string m_testFileName;
};

const int8_t kMinInt8 = std::numeric_limits<int8_t>::min();
const int8_t kMaxInt8 = std::numeric_limits<int8_t>::max();
const int16_t kMinInt16 = std::numeric_limits<int16_t>::min();
const int16_t kMaxInt16 = std::numeric_limits<int16_t>::max();

TEST_F(FormatWithChannels, Int16ToInt8)
{
	for (int fileChannels = 1; fileChannels <= 4; fileChannels++)
	{
		AFfilehandle file = createTestFile(16, fileChannels);
		const int frameCount = 3;
		int16_t data[frameCount * fileChannels];
		std::fill(data, data + frameCount * fileChannels, 0);
		data[0] = kMinInt16;
		data[2 * fileChannels] = kMaxInt16;

		AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
		ASSERT_EQ(framesWritten, frameCount);
		afCloseFile(file);

		file = openTestFile(8, 1);
		ASSERT_TRUE(file != NULL);
		int8_t readData[frameCount];
		AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
		ASSERT_EQ(framesRead, frameCount);
		afCloseFile(file);
		EXPECT_EQ(readData[0], kMinInt8);
		EXPECT_EQ(readData[1], 0);
		EXPECT_EQ(readData[2], kMaxInt8);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
