/*
	Copyright (C) 2011-2014, Michael Pruett. All rights reserved.

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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "TestUtilities.h"

struct Miscellaneous
{
	int id;
	int type;
	const char *data;
};

const Miscellaneous kMiscellaneous[] =
{
	{ 1, AF_MISC_COPY, "1806 Ludwig van Beethoven" },
	{ 2, AF_MISC_NAME, "Violin Concerto in D major" }
};

const int kNumMiscellaneous = sizeof (kMiscellaneous) / sizeof (Miscellaneous);

void writeMiscellaneous(int fileFormat, const std::string &testFileName)
{
	AFfilesetup setup = afNewFileSetup();
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitFileFormat(setup, fileFormat);
	int *miscIDs = new int[kNumMiscellaneous];
	for (int i=0; i<kNumMiscellaneous; i++)
		miscIDs[i] = kMiscellaneous[i].id;
	afInitMiscIDs(setup, miscIDs, kNumMiscellaneous);
	delete [] miscIDs;
	for (int i=0; i<kNumMiscellaneous; i++)
	{
		afInitMiscType(setup, kMiscellaneous[i].id, kMiscellaneous[i].type);
		afInitMiscSize(setup, kMiscellaneous[i].id, strlen(kMiscellaneous[i].data));
	}

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file);
	afFreeFileSetup(setup);
	for (int i=0; i<kNumMiscellaneous; i++)
	{
		int result = afWriteMisc(file, kMiscellaneous[i].id,
			kMiscellaneous[i].data,
			strlen(kMiscellaneous[i].data));
		EXPECT_EQ(strlen(kMiscellaneous[i].data), result);
	}

	const int16_t samples[] = { 1, 2, 3, 4 };
	const int sampleCount = sizeof (samples) / sizeof (samples[0]);
	EXPECT_EQ(sampleCount, afWriteFrames(file, AF_DEFAULT_TRACK, samples, sampleCount));
	ASSERT_EQ(0, afCloseFile(file));
}

void readMiscellaneous(const std::string &testFileName)
{
	AFfilehandle file = afOpenFile(testFileName.c_str(), "r", NULL);
	ASSERT_TRUE(file);
	int count = afGetMiscIDs(file, NULL);
	EXPECT_EQ(count, kNumMiscellaneous);
	int miscIDs[kNumMiscellaneous];
	afGetMiscIDs(file, miscIDs);
	for (int i=0; i<kNumMiscellaneous; i++)
		EXPECT_EQ(miscIDs[i], kMiscellaneous[i].id);
	for (int i=0; i<kNumMiscellaneous; i++)
	{
		int misctype = afGetMiscType(file, miscIDs[i]);
		EXPECT_EQ(misctype, kMiscellaneous[i].type);

		int datasize = afGetMiscSize(file, miscIDs[i]);
		EXPECT_EQ(datasize, strlen(kMiscellaneous[i].data));

		char *data = new char[datasize];
		EXPECT_EQ(datasize, afReadMisc(file, miscIDs[i], data, datasize));

		EXPECT_TRUE(!memcmp(data, kMiscellaneous[i].data, datasize));

		delete [] data;
	}
	ASSERT_EQ(0, afCloseFile(file));
}

void testMiscellaneous(int fileFormat)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Miscellaneous", &testFileName));
	writeMiscellaneous(fileFormat, testFileName);
	readMiscellaneous(testFileName);
	ASSERT_EQ(0, ::unlink(testFileName.c_str()));
}

TEST(Miscellaneous, AIFF) { testMiscellaneous(AF_FILE_AIFF); }
TEST(Miscellaneous, AIFFC) { testMiscellaneous(AF_FILE_AIFFC); }
TEST(Miscellaneous, WAVE) { testMiscellaneous(AF_FILE_WAVE); }
TEST(Miscellaneous, IFF_8SVX) { testMiscellaneous(AF_FILE_IFF_8SVX); }

void testMiscellaneousUnsupported(int fileFormat)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Miscellaneous", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitFileFormat(setup, fileFormat);
	int *miscIDs = new int[kNumMiscellaneous];
	for (int i=0; i<kNumMiscellaneous; i++)
		miscIDs[i] = kMiscellaneous[i].id;
	afInitMiscIDs(setup, miscIDs, kNumMiscellaneous);
	delete [] miscIDs;
	for (int i=0; i<kNumMiscellaneous; i++)
	{
		afInitMiscType(setup, kMiscellaneous[i].id, kMiscellaneous[i].type);
		afInitMiscSize(setup, kMiscellaneous[i].id, strlen(kMiscellaneous[i].data));
	}

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_FALSE(file);
	afFreeFileSetup(setup);

	ASSERT_EQ(0, ::unlink(testFileName.c_str()));
}

TEST(Miscellaneous, Raw) { testMiscellaneousUnsupported(AF_FILE_RAWDATA); }
TEST(Miscellaneous, NeXT) { testMiscellaneousUnsupported(AF_FILE_NEXTSND); }
TEST(Miscellaneous, IRCAM) { testMiscellaneousUnsupported(AF_FILE_IRCAM); }
TEST(Miscellaneous, AVR) { testMiscellaneousUnsupported(AF_FILE_AVR); }
TEST(Miscellaneous, SampleVision) { testMiscellaneousUnsupported(AF_FILE_SAMPLEVISION); }
TEST(Miscellaneous, VOC) { testMiscellaneousUnsupported(AF_FILE_VOC); }
TEST(Miscellaneous, NIST) { testMiscellaneousUnsupported(AF_FILE_NIST_SPHERE); }
TEST(Miscellaneous, CAF) { testMiscellaneousUnsupported(AF_FILE_CAF); }
TEST(Miscellaneous, FLAC) { testMiscellaneousUnsupported(AF_FILE_FLAC); }

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
