/*
	Audio File Library
	Copyright (C) 2010-2012, Michael Pruett <michael@68k.org>

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

#include "config.h"
#include "FileModule.h"

#include "File.h"
#include "Track.h"

#include <errno.h>
#include <string.h>

FileModule::FileModule(Mode mode, Track *track, File *fh, bool canSeek) :
	m_mode(mode),
	m_track(track),
	m_fh(fh),
	m_canSeek(canSeek)
{
	track->fpos_next_frame = track->fpos_first_frame;
	track->frames2ignore = 0;
}

ssize_t FileModule::read(void *data, size_t nbytes)
{
	ssize_t bytesRead = m_fh->read(data, nbytes);
	if (bytesRead > 0)
	{
		m_track->fpos_next_frame += bytesRead;
	}
	return bytesRead;
}

ssize_t FileModule::write(const void *data, size_t nbytes)
{
	ssize_t bytesWritten = m_fh->write(data, nbytes);
	if (bytesWritten > 0)
	{
		m_track->fpos_next_frame += bytesWritten;
		m_track->data_size += bytesWritten;
	}
	return bytesWritten;
}

off_t FileModule::seek(off_t offset)
{
	return m_fh->seek(offset, File::SeekFromBeginning);
}

off_t FileModule::tell()
{
	return m_fh->tell();
}

off_t FileModule::length()
{
	return m_fh->length();
}

void FileModule::reportReadError(AFframecount framesRead,
	AFframecount framesToRead)
{
	// Report error if we haven't already.
	if (!m_track->filemodhappy)
		return;

	_af_error(AF_BAD_READ,
		"file missing data -- read %jd frames, should be %jd",
		static_cast<intmax_t>(m_track->nextfframe),
		static_cast<intmax_t>(m_track->totalfframes));
	m_track->filemodhappy = false;
}

void FileModule::reportWriteError(AFframecount framesWritten,
	AFframecount framesToWrite)
{
	// Report error if we haven't already.
	if (!m_track->filemodhappy)
		return;

	if (framesWritten < 0)
	{
		// Signal I/O error.
		_af_error(AF_BAD_WRITE,
			"unable to write data (%s) -- wrote %jd out of %jd frames",
			strerror(errno),
			static_cast<intmax_t>(m_track->nextfframe),
			static_cast<intmax_t>(m_track->nextfframe + framesToWrite));
	}
	else
	{
		// Signal disk full error.
		_af_error(AF_BAD_WRITE,
			"unable to write data (disk full) -- "
			"wrote %jd out of %jd frames",
			static_cast<intmax_t>(m_track->nextfframe + framesWritten),
			static_cast<intmax_t>(m_track->nextfframe + framesToWrite));
	}

	m_track->filemodhappy = false;
}

int FileModule::bufferSize() const
{
	if (mode() == Compress)
		return outChunk()->frameCount * inChunk()->f.bytesPerFrame(true);
	else
		return inChunk()->frameCount * outChunk()->f.bytesPerFrame(true);
}
