/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.
	Copyright (C) 2010, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA  02110-1301  USA
*/

/*
	PCM.cpp - read and file write module for uncompressed data
*/

#include "config.h"
#include "PCM.h"

#include <assert.h>
#include <math.h>

#include "Compiler.h"
#include "FileModule.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "compression.h"
#include "util.h"

bool _af_pcm_format_ok (AudioFormat *f)
{
	assert(!isnan(f->pcm.slope));
	assert(!isnan(f->pcm.intercept));
	assert(!isnan(f->pcm.minClip));
	assert(!isnan(f->pcm.maxClip));

	return true;
}

class PCM : public FileModule
{
public:
	static PCM *createCompress(Track *track, File *fh, bool canSeek,
		bool headerless, AFframecount *chunkFrames);
	static PCM *createDecompress(Track *track, File *fh, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual const char *name() const OVERRIDE { return "pcm"; }
	virtual void runPull() OVERRIDE;
	virtual void reset2() OVERRIDE;
	virtual void runPush() OVERRIDE;
	virtual void sync1() OVERRIDE;
	virtual void sync2() OVERRIDE;

private:
	int m_bytesPerFrame;

	/* saved_fpos_next_frame and saved_nextfframe apply only to writing. */
	int m_saved_fpos_next_frame;
	int m_saved_nextfframe;

	PCM(Mode, Track *, File *, bool canSeek);
};

PCM::PCM(Mode mode, Track *track, File *fh, bool canSeek) :
	FileModule(mode, track, fh, canSeek),
	m_bytesPerFrame(track->f.bytesPerFrame(false)),
	m_saved_fpos_next_frame(-1),
	m_saved_nextfframe(-1)
{
	if (mode == Decompress)
		track->f.compressionParams = AU_NULL_PVLIST;
}

PCM *PCM::createCompress(Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkframes)
{
	return new PCM(Compress, track, fh, canSeek);
}

void PCM::runPush()
{
	AFframecount frames2write = m_inChunk->frameCount;
	AFframecount n;

	/*
		WARNING: due to the optimization explained at the end
		of arrangemodules(), the pcm file module cannot depend
		on the presence of the intermediate working buffer
		which _AFsetupmodules usually allocates for file
		modules in their input or output chunk (for reading or
		writing, respectively).

		Fortunately, the pcm module has no need for such a buffer.
	*/

	ssize_t bytesWritten = write(m_inChunk->buffer, m_bytesPerFrame * frames2write);
	n = bytesWritten >= 0 ? bytesWritten / m_bytesPerFrame : 0;

	if (n != frames2write)
		reportWriteError(n, frames2write);

	m_track->nextfframe += n;
	m_track->totalfframes = m_track->nextfframe;
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));
}

void PCM::sync1()
{
	m_saved_fpos_next_frame = m_track->fpos_next_frame;
	m_saved_nextfframe = m_track->nextfframe;
}

void PCM::sync2()
{
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));

	/* We can afford to seek because sync2 is rare. */
	m_track->fpos_after_data = tell();

	m_track->fpos_next_frame = m_saved_fpos_next_frame;
	m_track->nextfframe = m_saved_nextfframe;
}

PCM *PCM::createDecompress(Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkframes)
{
	return new PCM(Decompress, track, fh, canSeek);
}

void PCM::runPull()
{
	AFframecount framesToRead = m_outChunk->frameCount;

	/*
		WARNING: Due to the optimization explained at the end of
		arrangemodules(), the pcm file module cannot depend on
		the presence of the intermediate working buffer which
		_AFsetupmodules usually allocates for file modules in
		their input or output chunk (for reading or writing,
		respectively).

		Fortunately, the pcm module has no need for such a buffer.
	*/

	/*
		Limit the number of frames to be read to the number of
		frames left in the track.
	*/
	if (m_track->totalfframes != -1 &&
		m_track->nextfframe + framesToRead > m_track->totalfframes)
	{
		framesToRead = m_track->totalfframes - m_track->nextfframe;
	}

	ssize_t bytesRead = read(m_outChunk->buffer, m_bytesPerFrame * framesToRead);
	AFframecount framesRead = bytesRead >= 0 ? bytesRead / m_bytesPerFrame : 0;

	m_track->nextfframe += framesRead;
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));

	/*
		If we got EOF from read, then we return the actual amount read.

		Complain only if there should have been more frames in the file.
	*/

	if (framesRead != framesToRead && m_track->totalfframes != -1)
		reportReadError(framesRead, framesToRead);

	m_outChunk->frameCount = framesRead;
}

void PCM::reset2()
{
	m_track->fpos_next_frame = m_track->fpos_first_frame +
		m_bytesPerFrame * m_track->nextfframe;

	m_track->frames2ignore = 0;
}

FileModule *_AFpcminitcompress (Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	return PCM::createCompress(track, fh, canSeek, headerless, chunkFrames);
}

FileModule *_AFpcminitdecompress (Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	return PCM::createDecompress(track, fh, canSeek, headerless, chunkFrames);
}
