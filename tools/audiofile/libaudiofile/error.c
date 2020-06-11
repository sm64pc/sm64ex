/*
	Audio File Library
	Copyright (C) 1998, Michael Pruett <michael@68k.org>

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
	error.c

	This file contains the routines used in the Audio File Library's
	error handling.
*/

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "audiofile.h"

static void defaultErrorFunction (long error, const char *str);

static AFerrfunc errorFunction = defaultErrorFunction;

AFerrfunc afSetErrorHandler (AFerrfunc efunc)
{
	AFerrfunc	old;

	old = errorFunction;
	errorFunction = efunc;

	return old;
}

static void defaultErrorFunction (long error, const char *str)
{
	fprintf(stderr, "Audio File Library: ");
	fprintf(stderr, "%s", str);
	fprintf(stderr, " [error %ld]\n", error);
}

void _af_error (int errorCode, const char *fmt, ...)
{
	char	buf[1024];
	va_list	ap;

	va_start(ap, fmt);

	vsnprintf(buf, 1024, fmt, ap);

	va_end(ap);

	if (errorFunction != NULL)
		errorFunction(errorCode, buf);
}
