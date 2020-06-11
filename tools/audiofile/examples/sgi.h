/*
	Audio File Library

	Copyright 1998-1999, Michael Pruett <michael@68k.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
	sgi.h

	These routines are used in SGI-specific test programs.
*/

#ifndef SGI_H
#define SGI_H

#include <dmedia/audio.h>

/*
	Set the sample width of an audio configuration.
*/
void setwidth (ALconfig config, int width);

/*
	Set the sample format of an audio configuration.
*/
void setsampleformat (ALconfig config, int width);

/*
	Set the sample rate of an audio port.
*/
void setrate (ALport port, double rate);

/*
	Wait until the audio port has no more samples to play.
*/
void waitport (ALport port);

#endif
