/*
	Audio File Library
	Copyright (C) 1999, Elliot Lee <sopwith@redhat.com>

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
	af_vfs.h

	Virtual file operations for the Audio File Library.
*/

#ifndef AUDIOFILE_VFS_H
#define AUDIOFILE_VFS_H

#include <audiofile.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#define AFAPI __attribute__((visibility("default")))
#else
#define AFAPI
#endif

struct _AFvirtualfile
{
	ssize_t (*read) (AFvirtualfile *vfile, void *data, size_t nbytes);
	AFfileoffset (*length) (AFvirtualfile *vfile);
	ssize_t (*write) (AFvirtualfile *vfile, const void *data, size_t nbytes);
	void (*destroy) (AFvirtualfile *vfile);
	AFfileoffset (*seek) (AFvirtualfile *vfile, AFfileoffset offset, int is_relative);
	AFfileoffset (*tell) (AFvirtualfile *vfile);

	void *closure;
};

AFAPI AFvirtualfile *af_virtual_file_new (void);
AFAPI void af_virtual_file_destroy (AFvirtualfile *vfile);

#undef AFAPI

#ifdef __cplusplus
}
#endif

#endif
