/*
	Copyright (C) 2010, Michael Pruett. All rights reserved.

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
#include "File.h"

#include "Compiler.h"
#include "af_vfs.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

class FilePOSIX : public File
{
public:
	FilePOSIX(int fd, AccessMode mode) : File(mode), m_fd(fd) { }
	virtual ~FilePOSIX() { close(); }

	virtual int close() OVERRIDE;
	virtual ssize_t read(void *data, size_t nbytes) OVERRIDE;
	virtual ssize_t write(const void *data, size_t nbytes) OVERRIDE;
	virtual off_t length() OVERRIDE;
	virtual off_t seek(off_t offset, SeekOrigin origin) OVERRIDE;
	virtual off_t tell() OVERRIDE;

private:
	int m_fd;
};

class FileVF : public File
{
public:
	FileVF(AFvirtualfile *vf, AccessMode mode) : File(mode), m_vf(vf) { }
	virtual ~FileVF() { close(); }

	virtual int close() OVERRIDE;
	virtual ssize_t read(void *data, size_t nbytes) OVERRIDE;
	virtual ssize_t write(const void *data, size_t nbytes) OVERRIDE;
	virtual off_t length() OVERRIDE;
	virtual off_t seek(off_t offset, SeekOrigin origin) OVERRIDE;
	virtual off_t tell() OVERRIDE;

private:
	AFvirtualfile *m_vf;
};

File *File::open(const char *path, File::AccessMode mode)
{
	int flags = 0;
	if (mode == ReadAccess)
		flags = O_RDONLY;
	else if (mode == WriteAccess)
		flags = O_CREAT | O_WRONLY | O_TRUNC;
#if defined(WIN32) || defined(__CYGWIN__)
	flags |= O_BINARY;
#endif
	int fd = ::open(path, flags, 0666);
	if (fd == -1)
		return NULL;
	File *file = new FilePOSIX(fd, mode);
	return file;
}

File *File::create(int fd, File::AccessMode mode)
{
	return new FilePOSIX(fd, mode);
}

File *File::create(AFvirtualfile *vf, File::AccessMode mode)
{
	return new FileVF(vf, mode);
}

File::~File()
{
}

bool File::canSeek()
{
	return seek(0, File::SeekFromCurrent) != -1;
}

int FilePOSIX::close()
{
	if (m_fd == -1)
		return 0;

	int result = ::close(m_fd);
	m_fd = -1;
	return result;
}

ssize_t FilePOSIX::read(void *data, size_t nbytes)
{
	return ::read(m_fd, data, nbytes);
}

ssize_t FilePOSIX::write(const void *data, size_t nbytes)
{
	return ::write(m_fd, data, nbytes);
}

off_t FilePOSIX::length()
{
	off_t current = tell();
	if (current == -1)
		return -1;
	off_t length = seek(0, SeekFromEnd);
	if (length == -1)
		return -1;
	seek(current, SeekFromBeginning);
	return length;
}

off_t FilePOSIX::seek(off_t offset, File::SeekOrigin origin)
{
	int whence;
	switch (origin)
	{
		case SeekFromBeginning: whence = SEEK_SET; break;
		case SeekFromCurrent: whence = SEEK_CUR; break;
		case SeekFromEnd: whence = SEEK_END; break;
		default: assert(false); return -1;
	}
	return ::lseek(m_fd, offset, whence);
}

off_t FilePOSIX::tell()
{
	return seek(0, File::SeekFromCurrent);
}

int FileVF::close()
{
	if (m_vf)
		af_virtual_file_destroy(m_vf);
	m_vf = 0;
	return 0;
}

ssize_t FileVF::read(void *data, size_t nbytes)
{
	return m_vf->read(m_vf, data, nbytes);
}

ssize_t FileVF::write(const void *data, size_t nbytes)
{
	return m_vf->write(m_vf, data, nbytes);
}

off_t FileVF::length()
{
	return m_vf->length(m_vf);
}

off_t FileVF::seek(off_t offset, SeekOrigin origin)
{
	if (origin == SeekFromEnd)
		offset += length();
	return m_vf->seek(m_vf, offset, origin == SeekFromCurrent);
}

off_t FileVF::tell()
{
	return m_vf->tell(m_vf);
}
