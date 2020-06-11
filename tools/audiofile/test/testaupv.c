/*
	Audio File Library

	Copyright 1998, Michael Pruett <michael@68k.org>

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
	testaupv.c

	This is a program to test the AUpvlist commands.
*/

#ifdef __USE_SGI_HEADERS__
#include <dmedia/dm_audioutil.h>
#include <dmedia/audiofile.h>
#else
#include <audiofile.h>
#include <aupvlist.h>
#endif

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
	AUpvlist		list;
	int				size;
	AFfilehandle	file;

	long	fuck = 99;

	if (argc != 2)
	{
		fprintf(stderr, "usage: testaupv filename\n");
		exit(EXIT_FAILURE);
	}

	file = afOpenFile(argv[1], "r", NULL);

	list = AUpvnew(4);
	size = AUpvgetmaxitems(list);

	printf("AUpvsetparam: %d\n", AUpvsetparam(list, 0, AF_INST_MIDI_BASENOTE));
	printf("AUpvsetparam: %d\n", AUpvsetparam(list, 1, AF_INST_MIDI_LONOTE));
	printf("AUpvsetparam: %d\n", AUpvsetparam(list, 2, AF_INST_SUSLOOPID));
	printf("AUpvsetparam: %d\n", AUpvsetparam(list, 3, AF_INST_RELLOOPID));

	afGetInstParams(file, AF_DEFAULT_INST, list, 4);

	AUpvgetval(list, 0, &fuck);
	printf("AUpvgetval: %ld\n", fuck);

	AUpvgetval(list, 1, &fuck);
	printf("AUpvgetval: %ld\n", fuck);

	AUpvgetval(list, 2, &fuck);
	printf("AUpvgetval: %ld\n", fuck);

	AUpvgetval(list, 3, &fuck);
	printf("AUpvgetval: %ld\n", fuck);

	afCloseFile(file);

	return 0;
}
