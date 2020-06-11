/*
	Copyright (C) 2003, 2011, Michael Pruett. All rights reserved.

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
	This program tests seeking within an audio file.
*/

#include <audiofile.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "TestUtilities.h"

TEST(Seek, Seek)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Seek", &testFileName));

	const int kFrameCount = 2000;
	const int kPadFrameCount = kFrameCount + 5;
	const int kDataLength = kFrameCount * sizeof (int16_t);

	int16_t data[kFrameCount];
	int16_t readData[kPadFrameCount];

	AFfilesetup setup = afNewFileSetup();
	ASSERT_TRUE(setup);

	afInitFileFormat(setup, AF_FILE_AIFF);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file) << "could not open file for writing";

	afFreeFileSetup(setup);

	/* Initialize data to a nontrivial test pattern. */
	for (int i=0; i<kFrameCount; i++)
	{
		if ((i%2) != 0)
			data[i] = i;
		else
			data[i] = -i;
	}

	ASSERT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, data, kFrameCount),
		kFrameCount);

	afCloseFile(file);

	file = afOpenFile(testFileName.c_str(), "r", AF_NULL_FILESETUP);
	ASSERT_TRUE(file) << "Could not open file for reading";

	/*
		For each position in the file, seek to that position and
		read to the end of the file, checking that the data read
		matches the data written.
	*/
	for (int i=0; i<kFrameCount; i++)
	{
		memset(readData, 0, kDataLength);

		AFfileoffset currentposition = afSeekFrame(file, AF_DEFAULT_TRACK, i);
		ASSERT_EQ(currentposition, i) << "Incorrect seek position";

		AFframecount framesread = afReadFrames(file, AF_DEFAULT_TRACK,
			readData + i, kPadFrameCount);
		ASSERT_EQ(framesread, kFrameCount - i) <<
			"Incorrect number of frames read";

		ASSERT_TRUE(!memcmp(data + i, readData + i,
			framesread * sizeof (int16_t))) <<
			"Error in data read";
	}

	afCloseFile(file);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

int main (int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
