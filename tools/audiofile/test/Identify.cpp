/*
	Audio File Library
	Copyright (C) 2014, Michael Pruett <michael@68k.org>

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
#include <fcntl.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include "TestUtilities.h"

const uint8_t kNeXTData[] =
{
	'.', 's', 'n', 'd',
	0, 0, 0, 24, // offset of 24 bytes
	0, 0, 0, 2, // unspecified length
	0, 0, 0, 3, // 16-bit linear
	0, 0, 172, 68, // 44100 Hz
	0, 0, 0, 1, // 1 channel
	0, 0
};

TEST(Identify, NeXT)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Identify", &testFileName));
	int fd = ::open(testFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
	ASSERT_GT(fd, -1);
	ASSERT_EQ(::write(fd, kNeXTData, sizeof (kNeXTData)), sizeof (kNeXTData));

	EXPECT_TRUE(AF_FILE_NEXTSND == afIdentifyFD(fd));

	int implemented = -1;
	EXPECT_TRUE(AF_FILE_NEXTSND == afIdentifyNamedFD(fd, testFileName.c_str(), &implemented));
	EXPECT_EQ(implemented, 1);

	ASSERT_EQ(::close(fd), 0);
	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(Identify, Empty)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("Identify", &testFileName));
	int fd = ::open(testFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
	ASSERT_GT(fd, -1);

	EXPECT_TRUE(AF_FILE_UNKNOWN == afIdentifyFD(fd));

	int implemented = -1;
	EXPECT_TRUE(AF_FILE_UNKNOWN == afIdentifyNamedFD(fd, testFileName.c_str(), &implemented));
	EXPECT_EQ(implemented, 0);

	ASSERT_EQ(::close(fd), 0);
	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(Identify, NonSeekable)
{
	int pipefd[2];
	ASSERT_GE(::pipe(pipefd), 0);

	EXPECT_TRUE(AF_FILE_UNKNOWN == afIdentifyFD(pipefd[0]));
	EXPECT_TRUE(AF_FILE_UNKNOWN == afIdentifyNamedFD(pipefd[0], "", NULL));

	ASSERT_EQ(::close(pipefd[0]), 0);
	ASSERT_EQ(::close(pipefd[1]), 0);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
