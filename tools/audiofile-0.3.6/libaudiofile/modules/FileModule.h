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

#ifndef FILE_MODULE_H
#define FILE_MODULE_H

#include "Module.h"

class FileModule : public Module
{
public:
	virtual bool handlesSeeking() const { return false; }

	virtual int bufferSize() const;

protected:
	enum Mode { Compress, Decompress };
	FileModule(Mode, Track *, File *fh, bool canSeek);

	Mode mode() const { return m_mode; }
	bool canSeek() const { return m_canSeek; }

	ssize_t read(void *data, size_t nbytes);
	ssize_t write(const void *data, size_t nbytes);
	off_t seek(off_t offset);
	off_t tell();
	off_t length();

private:
	Mode m_mode;

protected:
	Track *m_track;

	void reportReadError(AFframecount framesRead, AFframecount framesRequested);
	void reportWriteError(AFframecount framesWritten, AFframecount framesRequested);

private:
	File *m_fh;
	bool m_canSeek;
};

#endif // FILE_MODULE_H
