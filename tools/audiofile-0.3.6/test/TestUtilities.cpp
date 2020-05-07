/*
	Audio File Library
	Copyright (C) 2012, Michael Pruett <michael@68k.org>

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

#include "TestUtilities.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool createTemporaryFile(const std::string &prefix, std::string *path)
{
	*path = "/tmp/" + prefix + "-XXXXXX";
	int fd = ::mkstemp(const_cast<char *>(path->c_str()));
	if (fd < 0)
		return false;
	::close(fd);
	return true;
}

bool createTemporaryFile(const char *prefix, char *path)
{
	snprintf(path, PATH_MAX, "/tmp/%s-XXXXXX", prefix);
	int fd = ::mkstemp(path);
	if (fd < 0)
		return false;
	::close(fd);
	return true;
}
