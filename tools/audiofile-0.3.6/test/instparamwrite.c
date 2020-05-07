/*
	Audio File Library

	Copyright 2000, Silicon Graphics, Inc.

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

#include <stdio.h>

#include <audiofile.h>

int main (int argc, char **argv)
{
	AFfilehandle	file;
	AFfilesetup	setup;
	AUpvlist	list;

	if (argc != 2)
	{
		fprintf(stderr, "usage: instparamwrite filename\n");
	}

	setup = afNewFileSetup();
	afInitFileFormat(setup, AF_FILE_AIFFC);

	file = afOpenFile(argv[1], "w", setup);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "could not open file %s for writing", argv[1]);
	}

	afFreeFileSetup(setup);

	/* Set the base note to a 'D.' */
	afSetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_BASENOTE, 50);

	/* Detune down by 30 cents. */
	afSetInstParamLong(file, AF_DEFAULT_INST, AF_INST_NUMCENTS_DETUNE, -30);

	afCloseFile(file);

	return 0;
}
