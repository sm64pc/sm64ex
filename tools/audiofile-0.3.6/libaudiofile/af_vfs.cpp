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
	af_vfs.cpp

	Virtual file operations for the Audio File Library.
*/

#include "config.h"

#include "afinternal.h"
#include "af_vfs.h"

#include <stdlib.h>

AFvirtualfile *af_virtual_file_new()
{
	return (AFvirtualfile *) calloc(sizeof (AFvirtualfile), 1);
}

void af_virtual_file_destroy(AFvirtualfile *vfile)
{
	vfile->destroy(vfile);

	free(vfile);
}
