/*
	Copyright (C) 2012, Michael Pruett. All rights reserved.

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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "TestUtilities.h"

void runTest(int fileFormat, int sampleFormat, int sampleWidth)
{
	IgnoreErrors ignoreErrors;

	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("InvalidSampleFormat", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, sampleFormat, sampleWidth);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	ASSERT_TRUE(afOpenFile(testFileName.c_str(), "w", setup) == AF_NULL_FILEHANDLE);
	afFreeFileSetup(setup);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

void testInt8(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_TWOSCOMP, 8);
}

void testUInt8(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_UNSIGNED, 8);
}

void testInt16(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_TWOSCOMP, 16);
}

void testUInt16(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_UNSIGNED, 16);
}

void testInt24(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_TWOSCOMP, 24);
}

void testUInt24(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_UNSIGNED, 24);
}

void testInt32(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_TWOSCOMP, 32);
}

void testUInt32(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_UNSIGNED, 32);
}

void testFloat32(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_FLOAT, 32);
}

void testFloat64(int fileFormat)
{
	runTest(fileFormat, AF_SAMPFMT_DOUBLE, 64);
}

TEST(AIFF, Float) { testFloat32(AF_FILE_AIFF); }
TEST(AIFF, Double) { testFloat64(AF_FILE_AIFF); }

TEST(IFF, Int16) { testInt16(AF_FILE_IFF_8SVX); }
TEST(IFF, Int24) { testInt24(AF_FILE_IFF_8SVX); }
TEST(IFF, Int32) { testInt32(AF_FILE_IFF_8SVX); }
TEST(IFF, Float) { testFloat32(AF_FILE_IFF_8SVX); }
TEST(IFF, Double) { testFloat64(AF_FILE_IFF_8SVX); }

TEST(AVR, Int24) { testInt24(AF_FILE_AVR); }
TEST(AVR, Int32) { testInt32(AF_FILE_AVR); }
TEST(AVR, Float) { testFloat32(AF_FILE_AVR); }
TEST(AVR, Double) { testFloat64(AF_FILE_AVR); }

TEST(SampleVision, Int8) { testInt8(AF_FILE_SAMPLEVISION); }
TEST(SampleVision, Int24) { testInt24(AF_FILE_SAMPLEVISION); }
TEST(SampleVision, Int32) { testInt32(AF_FILE_SAMPLEVISION); }
TEST(SampleVision, Float) { testFloat32(AF_FILE_SAMPLEVISION); }
TEST(SampleVision, Double) { testFloat64(AF_FILE_SAMPLEVISION); }

TEST(VOC, Int24) { testInt24(AF_FILE_VOC); }
TEST(VOC, Int32) { testInt32(AF_FILE_VOC); }
TEST(VOC, Float) { testFloat32(AF_FILE_VOC); }
TEST(VOC, Double) { testFloat64(AF_FILE_VOC); }

TEST(NIST, Int24) { testInt24(AF_FILE_NIST_SPHERE); }
TEST(NIST, Int32) { testInt32(AF_FILE_NIST_SPHERE); }
TEST(NIST, Float) { testFloat32(AF_FILE_NIST_SPHERE); }
TEST(NIST, Double) { testFloat64(AF_FILE_NIST_SPHERE); }

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
