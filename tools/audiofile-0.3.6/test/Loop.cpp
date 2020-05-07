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

/*
	Test that loops can be written to and read from an audio file.
*/

#include <audiofile.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include "TestUtilities.h"

TEST(Loop, AIFF)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Loop", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_AIFF);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);

	int markerIDs[] = { 1, 2, 3, 4 };
	const int numMarkers = sizeof (markerIDs) / sizeof (int);
	const char * const markerNames[numMarkers] =
	{
		"sustain loop start",
		"sustain loop end",
		"release loop start",
		"release loop end"
	};
	const int markerPositions[] = { 0, 2, 4, 5 };
	afInitMarkIDs(setup, AF_DEFAULT_TRACK, markerIDs, 4);
	for (int i=0; i<numMarkers; i++)
		afInitMarkName(setup, AF_DEFAULT_TRACK, markerIDs[i], markerNames[i]);

	int loopIDs[] = {1, 2};
	afInitLoopIDs(setup, AF_DEFAULT_INST, loopIDs, 2);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file) << "Could not open test file for writing";
	afFreeFileSetup(setup);

	const int frameCount = 5;
	int16_t frames[frameCount] = {0};

	EXPECT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, frames, frameCount),
		frameCount);

	for (int i=0; i<numMarkers; i++)
		afSetMarkPosition(file, AF_DEFAULT_TRACK, markerIDs[i], markerPositions[i]);

	afSetLoopStart(file, AF_DEFAULT_INST, 1, 1);
	afSetLoopEnd(file, AF_DEFAULT_INST, 1, 2);
	afSetLoopStart(file, AF_DEFAULT_INST, 2, 3);
	afSetLoopEnd(file, AF_DEFAULT_INST, 2, 4);

	afCloseFile(file);

	file = afOpenFile(testFileName.c_str(), "r", NULL);
	ASSERT_TRUE(file) << "Could not open test file for reading";

	ASSERT_EQ(afGetLoopIDs(file, AF_DEFAULT_INST, NULL), 2);
	int readLoopIDs[2];
	afGetLoopIDs(file, AF_DEFAULT_INST, readLoopIDs);
	for (int i=0; i<2; i++)
		ASSERT_EQ(loopIDs[i], readLoopIDs[i]);

	ASSERT_EQ(afGetMarkIDs(file, AF_DEFAULT_TRACK, NULL), numMarkers);
	int readMarkerIDs[numMarkers];
	afGetMarkIDs(file, AF_DEFAULT_TRACK, readMarkerIDs);
	for (int i=0; i<numMarkers; i++)
		ASSERT_EQ(markerIDs[i], readMarkerIDs[i]);
	
	for (int i=0; i<numMarkers; i++)
		ASSERT_TRUE(!strcmp(afGetMarkName(file, AF_DEFAULT_TRACK, markerIDs[i]),
			markerNames[i]));

	for (int i=0; i<numMarkers; i++)
		ASSERT_EQ(afGetMarkPosition(file, AF_DEFAULT_TRACK, markerIDs[i]),
			markerPositions[i]);

	ASSERT_EQ(afGetLoopStart(file, AF_DEFAULT_INST, 1), 1);
	ASSERT_EQ(afGetLoopEnd(file, AF_DEFAULT_INST, 1), 2);
	ASSERT_EQ(afGetLoopStart(file, AF_DEFAULT_INST, 2), 3);
	ASSERT_EQ(afGetLoopEnd(file, AF_DEFAULT_INST, 2), 4);

	ASSERT_EQ(afGetLoopTrack(file, AF_DEFAULT_INST, 1), AF_DEFAULT_TRACK);
	ASSERT_EQ(afGetLoopTrack(file, AF_DEFAULT_INST, 2), AF_DEFAULT_TRACK);

	ASSERT_EQ(afGetLoopStartFrame(file, AF_DEFAULT_INST, 1),
		markerPositions[0]);
	ASSERT_EQ(afGetLoopEndFrame(file, AF_DEFAULT_INST, 1),
		markerPositions[1]);
	ASSERT_EQ(afGetLoopStartFrame(file, AF_DEFAULT_INST, 2),
		markerPositions[2]);
	ASSERT_EQ(afGetLoopEndFrame(file, AF_DEFAULT_INST, 2),
		markerPositions[3]);

	afCloseFile(file);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
