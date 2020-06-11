/*
	Copyright (C) 2010, Michael Pruett. All rights reserved.

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

#include <gtest/gtest.h>
#include <audiofile.h>
#include <limits>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "TestUtilities.h"

class FloatToIntTest : public testing::Test
{
protected:
	virtual void SetUp()
	{
		ASSERT_TRUE(createTemporaryFile("FloatToInt", &m_testFileName));
	}
	virtual void TearDown()
	{
		ASSERT_EQ(::unlink(m_testFileName.c_str()), 0);
	}

	AFfilehandle createTestFile(int sampleWidth)
	{
		AFfilesetup setup = afNewFileSetup();
		afInitFileFormat(setup, AF_FILE_AIFFC);
		afInitChannels(setup, AF_DEFAULT_TRACK, 1);
		afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, sampleWidth);
		AFfilehandle file = afOpenFile(m_testFileName.c_str(), "w", setup);
		afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_FLOAT, 32);
		afFreeFileSetup(setup);
		return file;
	}
	AFfilehandle openTestFile(int sampleWidth)
	{
		AFfilehandle file = afOpenFile(m_testFileName.c_str(), "r", AF_NULL_FILESETUP);
		afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, sampleWidth);
		return file;
	}

private:
	std::string m_testFileName;
};

static const int8_t kMinInt8 = std::numeric_limits<int8_t>::min();
static const int8_t kMaxInt8 = std::numeric_limits<int8_t>::max();

TEST_F(FloatToIntTest, Int8)
{
	AFfilehandle file = createTestFile(8);
	const float data[] = { -1, 0, 1 };
	const int frameCount = sizeof (data) / sizeof (data[0]);
	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
	ASSERT_EQ(framesWritten, frameCount);
	afCloseFile(file);
	file = openTestFile(8);
	ASSERT_TRUE(file != NULL);
	int8_t readData[frameCount];
	AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
	ASSERT_EQ(framesRead, frameCount);
	afCloseFile(file);
	const int8_t expectedData[] = { kMinInt8, 0, kMaxInt8 };
	for (int i=0; i<frameCount; i++)
		EXPECT_EQ(readData[i], expectedData[i]);
}

static const int16_t kMinInt16 = std::numeric_limits<int16_t>::min();
static const int16_t kMaxInt16 = std::numeric_limits<int16_t>::max();

TEST_F(FloatToIntTest, Int16)
{
	AFfilehandle file = createTestFile(16);
	const float data[] = { -1, 0, 1 };
	const int frameCount = sizeof (data) / sizeof (data[0]);
	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
	ASSERT_EQ(framesWritten, frameCount);
	afCloseFile(file);
	file = openTestFile(16);
	ASSERT_TRUE(file != NULL);
	int16_t readData[frameCount];
	AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
	ASSERT_EQ(framesRead, frameCount);
	afCloseFile(file);
	const int16_t expectedData[] = { kMinInt16, 0, kMaxInt16 };
	for (int i=0; i<frameCount; i++)
		EXPECT_EQ(readData[i], expectedData[i]);
}

static const int32_t kMinInt24 = -1<<23;
static const int32_t kMaxInt24 = (1<<23) - 1;

TEST_F(FloatToIntTest, Int24)
{
	AFfilehandle file = createTestFile(24);
	const float data[] = { -1, 0, 1 };
	const int frameCount = sizeof (data) / sizeof (data[0]);
	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
	ASSERT_EQ(framesWritten, frameCount);
	afCloseFile(file);
	file = openTestFile(24);
	ASSERT_TRUE(file != NULL);
	int32_t readData[frameCount];
	AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
	ASSERT_EQ(framesRead, frameCount);
	afCloseFile(file);
	const int32_t expectedData[] = { kMinInt24, 0, kMaxInt24 };
	for (int i=0; i<frameCount; i++)
		EXPECT_EQ(readData[i], expectedData[i]);
}

static const int32_t kMinInt32 = std::numeric_limits<int32_t>::min();
static const int32_t kMaxInt32 = std::numeric_limits<int32_t>::max();

TEST_F(FloatToIntTest, Int32)
{
	AFfilehandle file = createTestFile(32);
	const float data[] = { -1, 0, 1 };
	const int frameCount = sizeof (data) / sizeof (data[0]);
	AFframecount framesWritten = afWriteFrames(file, AF_DEFAULT_TRACK, data, frameCount);
	ASSERT_EQ(framesWritten, frameCount);
	afCloseFile(file);
	file = openTestFile(32);
	ASSERT_TRUE(file != NULL);
	int32_t readData[frameCount];
	AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, readData, frameCount);
	ASSERT_EQ(framesRead, frameCount);
	afCloseFile(file);
	const int32_t expectedData[] = { kMinInt32, 0, kMaxInt32 };
	for (int i=0; i<frameCount; i++)
		EXPECT_EQ(readData[i], expectedData[i]);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
