/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>
	Copyright (C) 2000, Silicon Graphics, Inc.

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
	util.c

	This file contains general utility routines for the Audio File
	Library.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "audiofile.h"
#include "aupvlist.h"

#include "AudioFormat.h"
#include "File.h"
#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "aupvinternal.h"
#include "byteorder.h"
#include "compression.h"
#include "pcm.h"
#include "units.h"
#include "util.h"

/*
	_af_filesetup_ok and _af_filehandle_ok are sanity check routines
	which are called at the beginning of every external subroutine.
*/
bool _af_filesetup_ok (AFfilesetup setup)
{
	if (setup == AF_NULL_FILESETUP)
	{
		_af_error(AF_BAD_FILESETUP, "null file setup");
		return false;
	}
	if (setup->valid != _AF_VALID_FILESETUP)
	{
		_af_error(AF_BAD_FILESETUP, "invalid file setup");
		return false;
	}
	return true;
}

bool _af_filehandle_ok (AFfilehandle file)
{
	if (file == AF_NULL_FILEHANDLE)
	{
		_af_error(AF_BAD_FILEHANDLE, "null file handle");
		return false;
	}
	if (file->m_valid != _AF_VALID_FILEHANDLE)
	{
		_af_error(AF_BAD_FILEHANDLE, "invalid file handle");
		return false;
	}
	return true;
}

void *_af_malloc (size_t size)
{
	void	*p;

	if (size <= 0)
	{
		_af_error(AF_BAD_MALLOC, "bad memory allocation size request %zd", size);
		return NULL;
	}

	p = malloc(size);

#ifdef AF_DEBUG
	if (p)
		memset(p, 0xff, size);
#endif

	if (p == NULL)
	{
		_af_error(AF_BAD_MALLOC, "allocation of %zd bytes failed", size);
		return NULL;
	}

	return p;
}

char *_af_strdup (const char *s)
{
	char *p = (char *) malloc(strlen(s) + 1);

	if (p)
		strcpy(p, s);

	return p;
}

void *_af_realloc (void *p, size_t size)
{
	if (size <= 0)
	{
		_af_error(AF_BAD_MALLOC, "bad memory allocation size request %zd", size);
		return NULL;
	}

	p = realloc(p, size);

	if (p == NULL)
	{
		_af_error(AF_BAD_MALLOC, "allocation of %zd bytes failed", size);
		return NULL;
	}

	return p;
}

void *_af_calloc (size_t nmemb, size_t size)
{
	void	*p;

	if (nmemb <= 0 || size <= 0)
	{
		_af_error(AF_BAD_MALLOC, "bad memory allocation size request "
			"%zd elements of %zd bytes each", nmemb, size);
		return NULL;
	}

	p = calloc(nmemb, size);

	if (p == NULL)
	{
		_af_error(AF_BAD_MALLOC, "allocation of %zd bytes failed",
			nmemb*size);
		return NULL;
	}

	return p;
}

AUpvlist _af_pv_long (long val)
{
	AUpvlist	ret = AUpvnew(1);
	AUpvsetparam(ret, 0, 0);
	AUpvsetvaltype(ret, 0, AU_PVTYPE_LONG);
	AUpvsetval(ret, 0, &val);
	return ret;
}

AUpvlist _af_pv_double (double val)
{
	AUpvlist	ret = AUpvnew(1);
	AUpvsetparam(ret, 0, 0);
	AUpvsetvaltype(ret, 0, AU_PVTYPE_DOUBLE);
	AUpvsetval(ret, 0, &val);
	return ret;
}

AUpvlist _af_pv_pointer (void *val)
{
	AUpvlist	ret = AUpvnew(1);
	AUpvsetparam(ret, 0, 0);
	AUpvsetvaltype(ret, 0, AU_PVTYPE_PTR);
	AUpvsetval(ret, 0, &val);
	return ret;
}

bool _af_pv_getlong (AUpvlist pvlist, int param, long *l)
{
	for (int i=0; i<AUpvgetmaxitems(pvlist); i++)
	{
		int	p, t;

		AUpvgetparam(pvlist, i, &p);

		if (p != param)
			continue;

		AUpvgetvaltype(pvlist, i, &t);

		/* Ensure that this parameter is of type AU_PVTYPE_LONG. */
		if (t != AU_PVTYPE_LONG)
			return false;

		AUpvgetval(pvlist, i, l);
		return true;
	}

	return false;
}

bool _af_pv_getdouble (AUpvlist pvlist, int param, double *d)
{
	for (int i=0; i<AUpvgetmaxitems(pvlist); i++)
	{
		int	p, t;

		AUpvgetparam(pvlist, i, &p);

		if (p != param)
			continue;

		AUpvgetvaltype(pvlist, i, &t);

		/* Ensure that this parameter is of type AU_PVTYPE_DOUBLE. */
		if (t != AU_PVTYPE_DOUBLE)
			return false;

		AUpvgetval(pvlist, i, d);
		return true;
	}

	return false;
}

bool _af_pv_getptr (AUpvlist pvlist, int param, void **v)
{
	for (int i=0; i<AUpvgetmaxitems(pvlist); i++)
	{
		int	p, t;

		AUpvgetparam(pvlist, i, &p);

		if (p != param)
			continue;

		AUpvgetvaltype(pvlist, i, &t);

		/* Ensure that this parameter is of type AU_PVTYPE_PTR. */
		if (t != AU_PVTYPE_PTR)
			return false;

		AUpvgetval(pvlist, i, v);
		return true;
	}

	return false;
}

int _af_format_sample_size_uncompressed (const AudioFormat *format, bool stretch3to4)
{
	int	size = 0;

	switch (format->sampleFormat)
	{
		case AF_SAMPFMT_FLOAT:
			size = sizeof (float);
			break;
		case AF_SAMPFMT_DOUBLE:
			size = sizeof (double);
			break;
		default:
			size = (int) (format->sampleWidth + 7) / 8;
			if (format->compressionType == AF_COMPRESSION_NONE &&
				size == 3 && stretch3to4)
				size = 4;
			break;
	}

	return size;
}

float _af_format_sample_size (const AudioFormat *fmt, bool stretch3to4)
{
	const CompressionUnit *unit = _af_compression_unit_from_id(fmt->compressionType);
	float squishFactor = unit->squishFactor;

	return _af_format_sample_size_uncompressed(fmt, stretch3to4) /
		squishFactor;
}

int _af_format_frame_size_uncompressed (const AudioFormat *fmt, bool stretch3to4)
{
	return _af_format_sample_size_uncompressed(fmt, stretch3to4) *
		fmt->channelCount;
}

float _af_format_frame_size (const AudioFormat *fmt, bool stretch3to4)
{
	const CompressionUnit *unit = _af_compression_unit_from_id(fmt->compressionType);
	float squishFactor = unit->squishFactor;

	return _af_format_frame_size_uncompressed(fmt, stretch3to4) /
		squishFactor;
}

/*
	Set the sampleFormat and sampleWidth fields in f, and set the
	PCM info to the appropriate default values for the given sample
	format and sample width.
*/
status _af_set_sample_format (AudioFormat *f, int sampleFormat, int sampleWidth)
{
	switch (sampleFormat)
	{
		case AF_SAMPFMT_UNSIGNED:
		case AF_SAMPFMT_TWOSCOMP:
		if (sampleWidth < 1 || sampleWidth > 32)
		{
			_af_error(AF_BAD_SAMPFMT,
				"illegal sample width %d for integer data",
				sampleWidth);
			return AF_FAIL;
		}
		else
		{
			int bytes;

			f->sampleFormat = sampleFormat;
			f->sampleWidth = sampleWidth;

			bytes = _af_format_sample_size_uncompressed(f, false);

			if (sampleFormat == AF_SAMPFMT_TWOSCOMP)
				f->pcm = _af_default_signed_integer_pcm_mappings[bytes];
			else
				f->pcm = _af_default_unsigned_integer_pcm_mappings[bytes];
		}
		break;

		case AF_SAMPFMT_FLOAT:
			f->sampleFormat = sampleFormat;
			f->sampleWidth = 32;
			f->pcm = _af_default_float_pcm_mapping;
			break;
		case AF_SAMPFMT_DOUBLE:
			f->sampleFormat = sampleFormat;
			f->sampleWidth = 64;      /*for convenience */
			f->pcm = _af_default_double_pcm_mapping;
			break;
		default:
			_af_error(AF_BAD_SAMPFMT, "unknown sample format %d",
				sampleFormat);
			return AF_FAIL;
	}

	return AF_SUCCEED;
}

/*
	Verify the uniqueness of the nids ids given.

	idname is the name of what the ids identify, as in "loop"
	iderr is an error as in AF_BAD_LOOPID
*/
bool _af_unique_ids (const int *ids, int nids, const char *idname, int iderr)
{
	for (int i = 0; i < nids; i++)
	{
		for (int j = 0; j < i; j++)
		{
			if (ids[i] == ids[j])
			{
				_af_error(iderr, "nonunique %s id %d", idname, ids[i]);
				return false;
			}
		}
	}

	return true;
}
