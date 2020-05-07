/*
	Audio File Library
	Copyright (C) 2013 Michael Pruett <michael@68k.org>

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

#include <gtest/gtest.h>
#include <audiofile.h>
#include <stdlib.h>

#include "TestUtilities.h"

static void testInstrumentParameters(int fileFormat)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Instrument", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, fileFormat);
	int instrumentIDs[] = {AF_DEFAULT_INST};
	int numInstruments = sizeof (instrumentIDs) / sizeof (int);
	afInitInstIDs(setup, instrumentIDs, numInstruments);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file);

	afFreeFileSetup(setup);

	afSetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_BASENOTE, 50);
	afSetInstParamLong(file, AF_DEFAULT_INST, AF_INST_NUMCENTS_DETUNE, -30);
	afSetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_LOVELOCITY, 22);
	afSetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_HIVELOCITY, 111);

	ASSERT_EQ(0, afCloseFile(file));

	file = afOpenFile(testFileName.c_str(), "r", NULL);
	ASSERT_TRUE(file);
	ASSERT_EQ(fileFormat, afGetFileFormat(file, NULL));

	ASSERT_EQ(1, afGetInstIDs(file, NULL));
	int readInstrumentIDs[1] = {0};
	ASSERT_EQ(1, afGetInstIDs(file, readInstrumentIDs));
	ASSERT_EQ(AF_DEFAULT_INST, readInstrumentIDs[0]);

	EXPECT_EQ(50,
		afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_BASENOTE));
	EXPECT_EQ(-30,
		afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_NUMCENTS_DETUNE));
	EXPECT_EQ(22,
		afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_LOVELOCITY));
	EXPECT_EQ(111,
		afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_HIVELOCITY));

	ASSERT_EQ(0, afCloseFile(file));
	ASSERT_EQ(0, ::unlink(testFileName.c_str()));
}

TEST(Instrument, AIFF) { testInstrumentParameters(AF_FILE_AIFF); }
TEST(Instrument, AIFFC) { testInstrumentParameters(AF_FILE_AIFFC); }

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
