/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>

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
	util.h

	This file contains some general utility functions for the Audio
	File Library.
*/

#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdlib.h>

#include "audiofile.h"
#include "afinternal.h"

struct AudioFormat;

bool _af_filesetup_ok (AFfilesetup setup);
bool _af_filehandle_ok (AFfilehandle file);

void *_af_malloc (size_t size);
void *_af_realloc (void *ptr, size_t size);
void *_af_calloc (size_t nmemb, size_t size);
char *_af_strdup (const char *s);

AUpvlist _af_pv_long (long val);
AUpvlist _af_pv_double (double val);
AUpvlist _af_pv_pointer (void *val);

bool _af_pv_getlong (AUpvlist pvlist, int param, long *l);
bool _af_pv_getdouble (AUpvlist pvlist, int param, double *d);
bool _af_pv_getptr (AUpvlist pvlist, int param, void **v);

bool _af_unique_ids (const int *ids, int nids, const char *idname, int iderr);

float _af_format_frame_size (const AudioFormat *format, bool stretch3to4);
int _af_format_frame_size_uncompressed (const AudioFormat *format, bool stretch3to4);
float _af_format_sample_size (const AudioFormat *format, bool stretch3to4);
int _af_format_sample_size_uncompressed (const AudioFormat *format, bool stretch3to4);

status _af_set_sample_format (AudioFormat *f, int sampleFormat, int sampleWidth);

#endif
