/*
	Audio File Library
	Copyright (C) 2013 Michael Pruett <michael@68k.org>

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

#ifndef COMPILER_H
#define COMPILER_H

#if defined(__GNUC__) && !defined(__clang__)
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) \
	(GCC_VERSION >= (major * 10000 + minor * 100 + patch))
#if GCC_VERSION_AT_LEAST(4, 7, 0) && defined(__cplusplus) && __cplusplus >= 201103L
#define OVERRIDE override
#endif
#endif

#if defined(__clang__)
#if __has_extension(cxx_override_control)
#define OVERRRIDE override
#endif
#endif

#ifndef OVERRIDE
#define OVERRIDE
#endif

#endif
