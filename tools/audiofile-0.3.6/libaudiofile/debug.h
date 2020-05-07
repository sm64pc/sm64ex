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
	debug.h

	This header file declares debugging functions for the Audio
	File Library.
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "audiofile.h"
#include "afinternal.h"

void _af_print_filehandle (AFfilehandle filehandle);
void _af_print_tracks (AFfilehandle filehandle);
void _af_print_channel_matrix (double *matrix, int fchans, int vchans);
void _af_print_pvlist (AUpvlist list);

void _af_print_audioformat (AudioFormat *format);
void _af_print_frame (AFframecount frameno, double *frame, int nchannels,
	char *formatstring, int numberwidth,
	double slope, double intercept, double minclip, double maxclip);

#endif
