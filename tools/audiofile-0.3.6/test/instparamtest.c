/*
	Audio File Library

	Copyright 1998, Michael Pruett <michael@68k.org>
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

#include <audiofile.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
	AFfilehandle	file;
	long		result;
	int		count, instids;

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	file = afOpenFile(argv[1], "r", NULL);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "could not open file '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	count = afGetInstIDs(file, &instids);
	printf("%ld instruments in file '%s'\n", count, argv[1]);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_BASENOTE);
	printf("MIDI base note: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_NUMCENTS_DETUNE);
	printf("detune in cents: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_LONOTE);
	printf("MIDI low note: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_HINOTE);
	printf("MIDI high note: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_LOVELOCITY);
	printf("MIDI low velocity: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_HIVELOCITY);
	printf("MIDI high velocity: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_NUMDBS_GAIN);
	printf("gain in decibels: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_SUSLOOPID);
	printf("sustain loop id: %ld\n", result);

	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_RELLOOPID);
	printf("release loop id: %ld\n", result);

	afCloseFile(file);

	return 0;
}
