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
	int result;
	for (int i=0; i<kNumMiscellaneous; i++)
	{
		result = afWriteMisc(file, kMiscellaneous[i].id, kMiscellaneous[i].data,
			strlen(kMiscellaneous[i].data));
		EXPECT_EQ(result, strlen(kMiscellaneous[i].data));
	}

	const int16_t samples[] = { 1, 2, 3, 4 };
	afWriteFrames(file, AF_DEFAULT_TRACK, samples,
		sizeof (samples) / sizeof (samples[0]));
	afCloseFile(file);
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
		afReadMisc(file, miscIDs[i], data, datasize);

		EXPECT_TRUE(!memcmp(data, kMiscellaneous[i].data, datasize));

		delete [] data;
	}
	afCloseFile(file);
}

TEST(Miscellaneous, AIFF)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Miscellaneous", &testFileName));
	writeMiscellaneous(AF_FILE_AIFF, testFileName);
	readMiscellaneous(testFileName);
	::unlink(testFileName.c_str());
}

TEST(Miscellaneous, AIFFC)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Miscellaneous", &testFileName));
	writeMiscellaneous(AF_FILE_AIFFC, testFileName);
	readMiscellaneous(testFileName);
	::unlink(testFileName.c_str());
}

TEST(Miscellaneous, WAVE)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Miscellaneous", &testFileName));
	writeMiscellaneous(AF_FILE_WAVE, testFileName);
	readMiscellaneous(testFileName);
	::unlink(testFileName.c_str());
}

TEST(Miscellaneous, IFF_8SVX)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Miscellaneous", &testFileName));
	writeMiscellaneous(AF_FILE_IFF_8SVX, testFileName);
	readMiscellaneous(testFileName);
	::unlink(testFileName.c_str());
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
