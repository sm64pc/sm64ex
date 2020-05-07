/*
	Audio File Library
	Copyright (C) 2013, Michael Pruett <michael@68k.org>

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
#include <stdlib.h>
#include <unistd.h>

#include "TestUtilities.h"

TEST(SampleFormat, NullArguments)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("SampleFormat", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_AIFFC);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file);
	afFreeFileSetup(setup);

	int sampleFormat, sampleWidth;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	ASSERT_EQ(sampleFormat, AF_SAMPFMT_TWOSCOMP);
	ASSERT_EQ(sampleWidth, 16);

	sampleFormat = -1;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, NULL, &sampleWidth);
	ASSERT_EQ(sampleFormat, -1);
	ASSERT_EQ(sampleWidth, 16);

	sampleWidth = -1;
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, NULL);
	ASSERT_EQ(sampleFormat, AF_SAMPFMT_TWOSCOMP);
	ASSERT_EQ(sampleWidth, -1);

	afGetVirtualSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	ASSERT_EQ(sampleFormat, AF_SAMPFMT_TWOSCOMP);
	ASSERT_EQ(sampleWidth, 16);

	sampleFormat = -1;
	afGetVirtualSampleFormat(file, AF_DEFAULT_TRACK, NULL, &sampleWidth);
	ASSERT_EQ(sampleFormat, -1);
	ASSERT_EQ(sampleWidth, 16);

	sampleWidth = -1;
	afGetVirtualSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, NULL);
	ASSERT_EQ(sampleFormat, AF_SAMPFMT_TWOSCOMP);
	ASSERT_EQ(sampleWidth, -1);

	ASSERT_EQ(afCloseFile(file), 0);

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
