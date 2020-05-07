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

#include <audiofile.h>
#include <stdio.h>
#include <gtest/gtest.h>

struct ErrorListener *g_listener;

struct ErrorListener
{
	ErrorListener(long expectedError) :
		m_expectedError(expectedError),
		m_receivedError(-1),
		m_oldErrorFunction(0)
	{
		g_listener = this;
		m_oldErrorFunction = afSetErrorHandler(errorHandlerHelper);
	}
	~ErrorListener()
	{
		g_listener = 0;
		EXPECT_EQ(m_expectedError, m_receivedError);
		afSetErrorHandler(m_oldErrorFunction);
	}

	long m_expectedError;
	long m_receivedError;
	AFerrfunc m_oldErrorFunction;

	static void errorHandlerHelper(long error, const char *description)
	{
		g_listener->errorHandler(error, description);
	}
	void errorHandler(long error, const char *description)
	{
		m_receivedError = error;
		EXPECT_EQ(m_expectedError, m_receivedError);
	}
};

#define TEST_ERROR(ExpectedError, Message, TestBody) \
	{ \
		SCOPED_TRACE(Message); \
		ErrorListener el(ExpectedError); \
		TestBody; \
	}

TEST(Data, Null)
{
	TEST_ERROR(AF_BAD_FILEHANDLE, "closing null file handle",
		afCloseFile(AF_NULL_FILEHANDLE));

	TEST_ERROR(AF_BAD_FILEHANDLE, "reading from null file handle",
		afReadFrames(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL, 0));

	TEST_ERROR(AF_BAD_FILEHANDLE, "writing to null file handle",
		afWriteFrames(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL, 0));

	TEST_ERROR(AF_BAD_FILEHANDLE, "setting position on null file handle",
		afSeekFrame(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, 0));

	TEST_ERROR(AF_BAD_FILEHANDLE, "retrieving position on null file handle",
		afTellFrame(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));
}

TEST(Channels, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP, "initializing channels of null file setup",
		afInitChannels(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, 1));

	TEST_ERROR(AF_BAD_FILEHANDLE, "getting channels of null file handle",
		afGetChannels(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));

	TEST_ERROR(AF_BAD_FILEHANDLE, "getting virtual channels of null file handle",
		afGetVirtualChannels(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));

	TEST_ERROR(AF_BAD_FILEHANDLE, "setting virtual channels of null file handle",
		afSetVirtualChannels(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, 1));

	TEST_ERROR(AF_BAD_FILEHANDLE, "setting channel matrix of null file handle",
		afSetChannelMatrix(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL));
}

TEST(Rate, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP, "initializing rate of null file setup",
		afInitRate(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, 44100));

	TEST_ERROR(AF_BAD_FILEHANDLE, "getting rate of null file handle",
		afGetRate(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));
}

TEST(Compression, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP, "initializing compression of null file setup",
		afInitCompression(AF_NULL_FILESETUP, AF_DEFAULT_TRACK,
			AF_COMPRESSION_NONE));

	TEST_ERROR(AF_BAD_FILEHANDLE, "getting compression of null file handle",
		afGetCompression(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));
}

TEST(SampleFormat, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP,
		"initializing sample format of null file setup",
		afInitSampleFormat(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"getting sample format of null file handle",
		afGetSampleFormat(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL, NULL));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"getting virtual sample format of null file handle",
		afGetVirtualSampleFormat(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL, NULL));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"setting virtual sample format of null file handle",
		afSetVirtualSampleFormat(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16));
}

TEST(ByteOrder, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP,
		"initializing byte order of null file setup",
		afInitByteOrder(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"getting byte order of null file handle",
		afGetByteOrder(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"getting virtual byte order of null file handle",
		afGetVirtualByteOrder(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"setting virtual byte order of null file handle",
		afSetVirtualByteOrder(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN));
}

TEST(DataOffset, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP, "initializing data offset of null file setup",
		afInitDataOffset(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, 0));

	TEST_ERROR(AF_BAD_FILEHANDLE, "getting data offset of null file handle",
		afGetDataOffset(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));
}

TEST(FrameCount, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP, "initializing frame count of null file setup",
		afInitFrameCount(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, 0));

	TEST_ERROR(AF_BAD_FILEHANDLE, "getting frame count of null file handle",
		afGetFrameCount(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK));
}

TEST(AES, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP,
		"initializing AES channel data of null file setup",
		afInitAESChannelData(AF_NULL_FILESETUP, AF_DEFAULT_TRACK));

	TEST_ERROR(AF_BAD_FILESETUP,
		"initializing AES channel data of null file setup",
		afInitAESChannelDataTo(AF_NULL_FILESETUP, AF_DEFAULT_TRACK, 1));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"getting AES channel data of null file handle",
		afGetAESChannelData(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL));

	TEST_ERROR(AF_BAD_FILEHANDLE,
		"setting AES channel data of null file handle",
		afSetAESChannelData(AF_NULL_FILEHANDLE, AF_DEFAULT_TRACK, NULL));
}

TEST(Setup, Null)
{
	TEST_ERROR(AF_BAD_FILESETUP, "freeing null file setup",
		afFreeFileSetup(AF_NULL_FILESETUP));
}

TEST(File, Bad)
{
	TEST_ERROR(AF_BAD_OPEN, "opening nonexistent file for reading",
		afOpenFile("sldkjflsdkfjalksdjflaksdjflsakfdj", "r", NULL));

	TEST_ERROR(AF_BAD_ACCMODE, "opening file with null access mode",
		afOpenFile("", NULL, NULL));

	TEST_ERROR(AF_BAD_ACCMODE, "opening file with invalid access mode",
		afOpenFile("", "x", NULL));

	TEST_ERROR(AF_BAD_FILEFMT, "initializing file format to invalid value",
		AFfilesetup setup = afNewFileSetup();
		afInitFileFormat(setup, 91094);
		afFreeFileSetup(setup));

	TEST_ERROR(AF_BAD_SAMPFMT,
		"initializing sample format and sample width to invalid values",
		AFfilesetup setup = afNewFileSetup();
		afInitSampleFormat(setup, AF_DEFAULT_TRACK, 3992, 3932);
		afFreeFileSetup(setup));
}

TEST(Query, Bad)
{
	TEST_ERROR(AF_BAD_QUERY, "querying on bad selectors",
		afQueryLong(AF_QUERYTYPE_INST, 9999, 9999, 9999, 9999));

	TEST_ERROR(AF_BAD_QUERY, "querying on bad selectors",
		afQueryLong(AF_QUERYTYPE_INSTPARAM, 9999, 9999, 9999, 9999));

	TEST_ERROR(AF_BAD_QUERY, "querying on bad selectors",
		afQueryLong(AF_QUERYTYPE_FILEFMT, 9999, 9999, 9999, 9999));

	TEST_ERROR(AF_BAD_QUERY, "querying on bad selectors",
		afQueryLong(AF_QUERYTYPE_COMPRESSION, 9999, 9999, 9999, 9999));

	TEST_ERROR(AF_BAD_QUERY, "querying on bad selectors",
		afQueryLong(AF_QUERYTYPE_MARK, 9999, 9999, 9999, 9999));

	TEST_ERROR(AF_BAD_QUERYTYPE, "querying using bad query type",
		afQueryLong(9999, 9999, 9999, 9999, 9999));
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
