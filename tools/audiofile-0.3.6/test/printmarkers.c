/*
	Audio File Library

	Copyright (C) 2002, Silicon Graphics, Inc.

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
	printmarkers

	This program lists the markers in an audio file.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <audiofile.h>

void printtime (AFframecount position, double rate)
{
	double	time_in_seconds;

	time_in_seconds = (double) position / rate;

	/* Handle hours. */
	if (time_in_seconds > 3600)
	{
		printf("%d:", (int) (time_in_seconds / 3600));
		time_in_seconds = fmod(time_in_seconds, 3600);
	}

	/* Handle minutes. */
	if (time_in_seconds > 60)
	{
		printf("%02d:", (int) (time_in_seconds / 60));
		time_in_seconds = fmod(time_in_seconds, 60);
	}

	/* Handle seconds and milliseconds. */
	printf("%02.3f", time_in_seconds);
}

int main (int argc, char **argv)
{
	AFfilehandle	file;
	double		rate;
	int		markcount;
	int		*markids;
	int		i;

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		fprintf(stderr, "where filename is the name of an audio file containing markers\n");
		exit(0);
	}

	file = afOpenFile(argv[1], "r", NULL);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "Could not open file '%s' for reading.", argv[1]);
		exit(0);
	}

	markcount = afGetMarkIDs(file, AF_DEFAULT_TRACK, NULL);
	if (markcount <= 0)
	{
		fprintf(stderr, "The file '%s' does not contain any markers.", argv[1]);
		exit(0);
	}

	markids = calloc(markcount, sizeof (int));
	if (markids == NULL)
	{
		fprintf(stderr, "Could not allocate enough memory for markers.");
		exit(0);
	}

	afGetMarkIDs(file, AF_DEFAULT_TRACK, markids);

	rate = afGetRate(file, AF_DEFAULT_TRACK);

	for (i=0; i<markcount; i++)
	{
		AFframecount	position;
		const char	*name, *comment;

		position = afGetMarkPosition(file, AF_DEFAULT_TRACK, markids[i]);

		name = afGetMarkName(file, AF_DEFAULT_TRACK, markids[i]);
		comment = afGetMarkComment(file, AF_DEFAULT_TRACK, markids[i]);

		printf("marker %d, position %lld, time ", markids[i], position);

		printtime(position, rate);

		printf("\n");

		if (name != NULL)
			printf("\tname: %s\n", name);
		if (comment != NULL)
			printf("\tcomment: %s\n", comment);
	}

	afCloseFile(file);

	return EXIT_SUCCESS;
}
