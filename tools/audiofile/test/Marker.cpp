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
#include <unistd.h>

#include "TestUtilities.h"

static void testMarkers(int fileFormat, bool supportsComments)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Marker", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);

	const int markerIDs[] = { 1, 2, 3, 5, 8 };
	const int numMarkers = sizeof (markerIDs) / sizeof (int);
	const char * const markerNames[numMarkers] =
	{
		"one",
		"two",
		"three",
		"five",
		"eight"
	};
	const char * const markerComments[numMarkers] =
	{
		"comment one",
		"comment two",
		"comment three",
		"comment five",
		"comment eight"
	};
	const int markerPositions[numMarkers] = { 1, 2, 3, 5, 8 };
	afInitMarkIDs(setup, AF_DEFAULT_TRACK, markerIDs, numMarkers);
	for (int i=0; i<numMarkers; i++)
		afInitMarkName(setup, AF_DEFAULT_TRACK, markerIDs[i], markerNames[i]);

	for (int i=0; i<numMarkers; i++)
		afInitMarkComment(setup, AF_DEFAULT_TRACK, markerIDs[i], markerComments[i]);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file) << "Could not open test file for writing";
	afFreeFileSetup(setup);

	const int frameCount = 10;
	int16_t frames[frameCount] = {0};

	EXPECT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, frames, frameCount),
		frameCount);

	for (int i=0; i<numMarkers; i++)
		afSetMarkPosition(file, AF_DEFAULT_TRACK, markerIDs[i], markerPositions[i]);

	ASSERT_EQ(afCloseFile(file), 0);

	file = afOpenFile(testFileName.c_str(), "r", NULL);
	ASSERT_TRUE(file) << "Could not open test file for reading";
	EXPECT_EQ(afGetFileFormat(file, NULL), fileFormat);

	ASSERT_EQ(afGetMarkIDs(file, AF_DEFAULT_TRACK, NULL), numMarkers);
	int readMarkerIDs[numMarkers];
	afGetMarkIDs(file, AF_DEFAULT_TRACK, readMarkerIDs);
	for (int i=0; i<numMarkers; i++)
		EXPECT_EQ(markerIDs[i], readMarkerIDs[i]);
	
	for (int i=0; i<numMarkers; i++)
		EXPECT_TRUE(!strcmp(afGetMarkName(file, AF_DEFAULT_TRACK, markerIDs[i]),
			markerNames[i]));

	for (int i=0; i<numMarkers; i++)
	{
		const char *comment = afGetMarkComment(file, AF_DEFAULT_TRACK,
			markerIDs[i]);
		if (supportsComments)
			EXPECT_TRUE(!strcmp(comment, markerComments[i]));
	}

	for (int i=0; i<numMarkers; i++)
		EXPECT_EQ(afGetMarkPosition(file, AF_DEFAULT_TRACK, markerIDs[i]),
			markerPositions[i]);

	ASSERT_EQ(afCloseFile(file), 0);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(Marker, AIFF)
{
	testMarkers(AF_FILE_AIFF, false);
}

TEST(Marker, AIFC)
{
	testMarkers(AF_FILE_AIFFC, false);
}

TEST(Marker, WAVE)
{
	testMarkers(AF_FILE_WAVE, true);
}

static void testUnsupported(int fileFormat)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Marker", &testFileName));

	AFerrfunc errorHandler = afSetErrorHandler(NULL);

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);

	const int markerIDs[] = { 1 };
	const int numMarkers = sizeof (markerIDs) / sizeof (int);
	const char * const markerNames[numMarkers] = { "name" };
	const char * const markerComments[numMarkers] = { "comment" };
	afInitMarkIDs(setup, AF_DEFAULT_TRACK, markerIDs, numMarkers);
	for (int i=0; i<numMarkers; i++)
		afInitMarkName(setup, AF_DEFAULT_TRACK, markerIDs[i], markerNames[i]);

	for (int i=0; i<numMarkers; i++)
		afInitMarkComment(setup, AF_DEFAULT_TRACK, markerIDs[i], markerComments[i]);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_FALSE(file) << "Did not expect opening test file for writing to succeed";
	afFreeFileSetup(setup);

	afSetErrorHandler(errorHandler);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(Unsupported, Raw) { testUnsupported(AF_FILE_RAWDATA); }
TEST(Unsupported, NeXT) { testUnsupported(AF_FILE_NEXTSND); }
TEST(Unsupported, IRCAM) { testUnsupported(AF_FILE_IRCAM); }
TEST(Unsupported, AVR) { testUnsupported(AF_FILE_AVR); }
TEST(Unsupported, IFF) { testUnsupported(AF_FILE_IFF_8SVX); }
TEST(Unsupported, SampleVision) { testUnsupported(AF_FILE_SAMPLEVISION); }
TEST(Unsupported, VOC) { testUnsupported(AF_FILE_VOC); }
TEST(Unsupported, NIST) { testUnsupported(AF_FILE_NIST_SPHERE); }
TEST(Unsupported, CAF) { testUnsupported(AF_FILE_CAF); }
TEST(Unsupported, FLAC) { testUnsupported(AF_FILE_FLAC); }

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
