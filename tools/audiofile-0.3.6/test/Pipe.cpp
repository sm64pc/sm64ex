/*
	Copyright (C) 2012, Michael Pruett. All rights reserved.

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

#include <stdlib.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <audiofile.h>

TEST(Pipe, Pipe)
{
	const int kFrameCount = 500;
	const int kSampleCount = 2 * kFrameCount;

	int16_t data[kSampleCount];
	int16_t readData[kSampleCount];

	for (int i=0; i<kSampleCount; i++)
		data[i] = 3*i + 2;

	AFfilesetup setup = afNewFileSetup();
	ASSERT_TRUE(setup);

	afInitFileFormat(setup, AF_FILE_RAWDATA);
	afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
	afInitChannels(setup, AF_DEFAULT_TRACK, 2);
	afInitRate(setup, AF_DEFAULT_TRACK, 44100);

	int pipefd[2];
	ASSERT_GE(::pipe(pipefd), 0);

	AFfilehandle handle;

	// Write into pipe.
	handle = afOpenFD(pipefd[1], "w", setup);
	ASSERT_TRUE(handle);
	AFframecount framesWritten;
	framesWritten = afWriteFrames(handle, AF_DEFAULT_TRACK, data, kFrameCount);
	ASSERT_EQ(framesWritten, kFrameCount);
	ASSERT_EQ(afCloseFile(handle), 0);

	// Read from pipe.
	handle = afOpenFD(pipefd[0], "r", setup);
	ASSERT_TRUE(handle);
	AFframecount framesRead;
	framesRead = afReadFrames(handle, AF_DEFAULT_TRACK, readData, kFrameCount);
	ASSERT_EQ(framesRead, kFrameCount);
	ASSERT_TRUE(!::memcmp(readData, data, kFrameCount * sizeof (int16_t))) <<
		"Data read does not match data written";
	ASSERT_EQ(afCloseFile(handle), 0);

	afFreeFileSetup(setup);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
