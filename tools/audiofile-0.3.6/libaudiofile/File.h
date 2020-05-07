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

#ifndef FILE_H
#define FILE_H

#include "Shared.h"
#include <sys/types.h>

typedef struct _AFvirtualfile AFvirtualfile;

class File : public Shared<File>
{
public:
	enum AccessMode
	{
		ReadAccess,
		WriteAccess
	};

	enum SeekOrigin
	{
		SeekFromBeginning,
		SeekFromCurrent,
		SeekFromEnd
	};

	static File *open(const char *path, AccessMode mode);
	static File *create(int fd, AccessMode mode);
	static File *create(AFvirtualfile *vf, AccessMode mode);

	virtual ~File();
	virtual int close() = 0;
	virtual ssize_t read(void *data, size_t nbytes) = 0;
	virtual ssize_t write(const void *data, size_t nbytes) = 0;
	virtual off_t length() = 0;
	virtual off_t seek(off_t offset, SeekOrigin origin) = 0;
	virtual off_t tell() = 0;

	bool canSeek();

	AccessMode accessMode() const { return m_accessMode; }

private:
	AccessMode m_accessMode;

protected:
	File(AccessMode mode) : m_accessMode(mode) { }
};

#endif // FILE_H
