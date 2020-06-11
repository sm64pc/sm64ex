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
#include <unistd.h>

#include "TestUtilities.h"

TEST(Query, FileFormats)
{
	IgnoreErrors ignoreErrors;

	int fileFormats[] =
	{
		AF_FILE_UNKNOWN,
		AF_FILE_RAWDATA,
		AF_FILE_AIFFC,
		AF_FILE_AIFF,
		AF_FILE_NEXTSND,
		AF_FILE_WAVE,
		AF_FILE_BICSF,
		AF_FILE_MPEG1BITSTREAM,
		AF_FILE_SOUNDDESIGNER1,
		AF_FILE_SOUNDDESIGNER2,
		AF_FILE_AVR,
		AF_FILE_IFF_8SVX,
		AF_FILE_SAMPLEVISION,
		AF_FILE_VOC,
		AF_FILE_NIST_SPHERE,
		AF_FILE_SOUNDFONT2,
		AF_FILE_CAF
	};
	int numFileFormats = sizeof (fileFormats) / sizeof (int);
	for (int i=0; i<numFileFormats; i++)
	{
		long implemented = afQueryLong(AF_QUERYTYPE_FILEFMT,
			AF_QUERY_IMPLEMENTED, fileFormats[i], 0, 0);
		const char *label =
			static_cast<const char *>(afQueryPointer(AF_QUERYTYPE_FILEFMT,
				AF_QUERY_LABEL, fileFormats[i], 0, 0));
		if (implemented)
			EXPECT_TRUE(label);
		const char *name =
			static_cast<const char *>(afQueryPointer(AF_QUERYTYPE_FILEFMT,
				AF_QUERY_NAME, fileFormats[i], 0, 0));
		if (implemented)
			EXPECT_TRUE(name);
		const char *description =
			static_cast<const char *>(afQueryPointer(AF_QUERYTYPE_FILEFMT,
				AF_QUERY_DESC, fileFormats[i], 0, 0));
		if (implemented)
			EXPECT_TRUE(description);
	}
}

TEST(Query, CompressionFormats)
{
	IgnoreErrors ignoreErrors;

	int compressionFormats[] =
	{
		AF_COMPRESSION_NONE,
		AF_COMPRESSION_G711_ULAW,
		AF_COMPRESSION_G711_ALAW,
		AF_COMPRESSION_IMA,
		AF_COMPRESSION_MS_ADPCM,
		AF_COMPRESSION_G722,
		AF_COMPRESSION_APPLE_ACE2,
		AF_COMPRESSION_APPLE_ACE8,
		AF_COMPRESSION_APPLE_MAC3,
		AF_COMPRESSION_APPLE_MAC6
	};
	int numCompressionFormats = sizeof (compressionFormats) / sizeof (int);
	for (int i=0; i<numCompressionFormats; i++)
	{
		long implemented = afQueryLong(AF_QUERYTYPE_COMPRESSION,
			AF_QUERY_IMPLEMENTED, compressionFormats[i], 0, 0);
		long nativeSampleFormat = afQueryLong(AF_QUERYTYPE_COMPRESSION,
			AF_QUERY_NATIVE_SAMPFMT, compressionFormats[i], 0, 0);
		if (implemented)
			EXPECT_TRUE(nativeSampleFormat == AF_SAMPFMT_TWOSCOMP ||
				nativeSampleFormat == AF_SAMPFMT_UNSIGNED ||
				nativeSampleFormat == AF_SAMPFMT_FLOAT ||
				nativeSampleFormat == AF_SAMPFMT_DOUBLE);
		long nativeSampleWidth = afQueryLong(AF_QUERYTYPE_COMPRESSION,
			AF_QUERY_NATIVE_SAMPWIDTH, compressionFormats[i], 0, 0);
		if (implemented)
		{
			EXPECT_GE(nativeSampleWidth, 1);
			EXPECT_LE(nativeSampleWidth, 64);
		}
		const char *label =
			static_cast<const char *>(afQueryPointer(AF_QUERYTYPE_COMPRESSION,
				AF_QUERY_LABEL, compressionFormats[i], 0, 0));
		if (implemented)
			EXPECT_TRUE(label);
		const char *name =
			static_cast<const char *>(afQueryPointer(AF_QUERYTYPE_COMPRESSION,
				AF_QUERY_NAME, compressionFormats[i], 0, 0));
		if (implemented)
			EXPECT_TRUE(name);
		const char *description =
			static_cast<const char *>(afQueryPointer(AF_QUERYTYPE_COMPRESSION,
				AF_QUERY_DESC, compressionFormats[i], 0, 0));
		if (implemented)
			EXPECT_TRUE(description);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
