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

#include "config.h"

#include <af_vfs.h>
#include <audiofile.h>
#include <gtest/gtest.h>
#include <limits>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "TestUtilities.h"

TEST(VirtualFile, Basic)
{
	ASSERT_GE(sizeof (off_t), 8) << "Size of off_t must be at least 8 bytes.";
}

static ssize_t vf_read(AFvirtualfile *vf, void *data, size_t nbytes)
{
	FILE *fp = static_cast<FILE *>(vf->closure);
	return fread(data, 1, nbytes, fp);
}

static AFfileoffset vf_length(AFvirtualfile *vf)
{
	FILE *fp = static_cast<FILE *>(vf->closure);
	off_t current = ftello(fp);
	fseeko(fp, 0, SEEK_END);
	off_t length = ftello(fp);
	fseeko(fp, current, SEEK_SET);
	return length;
}

static ssize_t vf_write(AFvirtualfile *vf, const void *data, size_t nbytes)
{
	FILE *fp = static_cast<FILE *>(vf->closure);
	return fwrite(data, 1, nbytes, fp);
}

static void vf_close(AFvirtualfile *vf)
{
	FILE *fp = static_cast<FILE *>(vf->closure);
	fclose(fp);
}

static AFfileoffset vf_seek(AFvirtualfile *vf, AFfileoffset offset, int is_relative)
{
	FILE *fp = static_cast<FILE *>(vf->closure);
	fseeko(fp, offset, is_relative ? SEEK_CUR : SEEK_SET);
	return ftello(fp);
}

static AFfileoffset vf_tell(AFvirtualfile *vf)
{
	FILE *fp = static_cast<FILE *>(vf->closure);
	return ftello(fp);
}

static AFvirtualfile *vf_create(FILE *fp)
{
	AFvirtualfile *vf = af_virtual_file_new();
	vf->read = vf_read;
	vf->length = vf_length;
	vf->write = vf_write;
	vf->destroy = vf_close;
	vf->seek = vf_seek;
	vf->tell = vf_tell;
	vf->closure = fp;
	return vf;
}

TEST(VirtualFile, ReadVirtual)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("VirtualFile", &testFileName));

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_AIFF);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	AFfilehandle file = afOpenFile(testFileName.c_str(), "w", setup);
	ASSERT_TRUE(file) << "Could not open virtual file";

	afFreeFileSetup(setup);

	const int16_t samples[] = { 1, 1, 2, 3, 5, 8, 13 };
	const int numSamples = sizeof (samples) / sizeof (int16_t);
	ASSERT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, samples, numSamples),
		numSamples) << "Incorrect number of samples written";

	ASSERT_EQ(afCloseFile(file), 0) << "Error closing virtual file";

	FILE *fp = fopen(testFileName.c_str(), "r");
	AFvirtualfile *vf = vf_create(fp);

	file = afOpenVirtualFile(vf, "r", NULL);
	ASSERT_TRUE(file) << "Could not open file";

	int16_t readSamples[numSamples];
	ASSERT_EQ(afReadFrames(file, AF_DEFAULT_TRACK, readSamples, numSamples),
		numSamples) << "Incorrect number of samples read";

	for (int i=0; i<numSamples; i++)
		ASSERT_EQ(samples[i], readSamples[i]);

	ASSERT_EQ(afCloseFile(file), 0) << "Error closing file";

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

TEST(VirtualFile, WriteVirtual)
{
	std::string testFileName;
	ASSERT_TRUE(createTemporaryFile("VirtualFile", &testFileName));

	FILE *fp = fopen(testFileName.c_str(), "w");
	AFvirtualfile *vf = vf_create(fp);

	AFfilesetup setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_AIFF);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	AFfilehandle file = afOpenVirtualFile(vf, "w", setup);
	ASSERT_TRUE(file) << "Could not open virtual file";

	afFreeFileSetup(setup);

	const int16_t samples[] = { 1, 1, 2, 3, 5, 8, 13 };
	const int numSamples = sizeof (samples) / sizeof (int16_t);
	ASSERT_EQ(afWriteFrames(file, AF_DEFAULT_TRACK, samples, numSamples),
		numSamples) << "Incorrect number of samples written";

	ASSERT_EQ(afCloseFile(file), 0) << "Error closing virtual file";

	file = afOpenFile(testFileName.c_str(), "r", NULL);
	ASSERT_TRUE(file) << "Could not open file";

	int16_t readSamples[numSamples];
	ASSERT_EQ(afReadFrames(file, AF_DEFAULT_TRACK, readSamples, numSamples),
		numSamples) << "Incorrect number of samples read";

	for (int i=0; i<numSamples; i++)
		ASSERT_EQ(samples[i], readSamples[i]);

	ASSERT_EQ(afCloseFile(file), 0) << "Error closing file";

	ASSERT_EQ(::unlink(testFileName.c_str()), 0);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
