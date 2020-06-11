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
	Marker.cpp

	This file contains routines for dealing with loop markers.
*/

#include "config.h"
#include "Marker.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "FileHandle.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "util.h"

void afInitMarkIDs(AFfilesetup setup, int trackid, const int *markids, int nmarks)
{
	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	if (track->markers != NULL)
	{
		for (int i=0; i<track->markerCount; i++)
		{
			if (track->markers[i].name != NULL)
				free(track->markers[i].name);
			if (track->markers[i].comment != NULL)
				free(track->markers[i].comment);
		}
		free(track->markers);
	}

	track->markers = (MarkerSetup *) _af_calloc(nmarks, sizeof (struct MarkerSetup));
	track->markerCount = nmarks;

	for (int i=0; i<nmarks; i++)
	{
		track->markers[i].id = markids[i];
		track->markers[i].name = _af_strdup("");
		track->markers[i].comment = _af_strdup("");
	}

	track->markersSet = true;
}

void afInitMarkName(AFfilesetup setup, int trackid, int markid,
	const char *namestr)
{
	int	markno;
	int	length;

	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	for (markno=0; markno<track->markerCount; markno++)
	{
		if (track->markers[markno].id == markid)
			break;
	}

	if (markno == track->markerCount)
	{
		_af_error(AF_BAD_MARKID, "no marker id %d for file setup", markid);
		return;
	}

	length = strlen(namestr);
	if (length > 255)
	{
		_af_error(AF_BAD_STRLEN,
			"warning: marker name truncated to 255 characters");
		length = 255;
	}

	if (track->markers[markno].name)
		free(track->markers[markno].name);
	if ((track->markers[markno].name = (char *) _af_malloc(length+1)) == NULL)
		return;
	strncpy(track->markers[markno].name, namestr, length);
	/*
		The null terminator is not set by strncpy if
		strlen(namestr) > length.  Set it here.
	*/
	track->markers[markno].name[length] = '\0';
}

void afInitMarkComment(AFfilesetup setup, int trackid, int markid,
	const char *commstr)
{
	int	markno;
	int	length;

	if (!_af_filesetup_ok(setup))
		return;

	TrackSetup *track = setup->getTrack(trackid);
	if (!track)
		return;

	for (markno=0; markno<track->markerCount; markno++)
	{
		if (track->markers[markno].id == markid)
			break;
	}

	if (markno == track->markerCount)
	{
		_af_error(AF_BAD_MARKID, "no marker id %d for file setup", markid);
		return;
	}

	length = strlen(commstr);

	if (track->markers[markno].comment)
		free(track->markers[markno].comment);
	if ((track->markers[markno].comment = (char *) _af_malloc(length+1)) == NULL)
		return;
	strcpy(track->markers[markno].comment, commstr);
}

char *afGetMarkName (AFfilehandle file, int trackid, int markid)
{
	if (!_af_filehandle_ok(file))
		return NULL;

	Track *track = file->getTrack(trackid);
	if (!track)
		return NULL;

	Marker *marker = track->getMarker(markid);
	if (!marker)
		return NULL;

	return marker->name;
}

char *afGetMarkComment (AFfilehandle file, int trackid, int markid)
{
	if (!_af_filehandle_ok(file))
		return NULL;

	Track *track = file->getTrack(trackid);
	if (!track)
		return NULL;

	Marker *marker = track->getMarker(markid);
	if (!marker)
		return NULL;

	return marker->comment;
}

void afSetMarkPosition (AFfilehandle file, int trackid, int markid,
	AFframecount position)
{
	if (!_af_filehandle_ok(file))
		return;

	if (!file->checkCanWrite())
		return;

	Track *track = file->getTrack(trackid);
	if (!track)
		return;

	Marker *marker = track->getMarker(markid);
	if (!marker)
		return;

	if (position < 0)
	{
		_af_error(AF_BAD_MARKPOS, "invalid marker position %jd",
			static_cast<intmax_t>(position));
		position = 0;
	}

	marker->position = position;
}

int afGetMarkIDs (AFfilehandle file, int trackid, int markids[])
{
	if (!_af_filehandle_ok(file))
		return -1;

	Track *track = file->getTrack(trackid);
	if (!track)
		return -1;

	if (markids != NULL)
	{
		for (int i=0; i<track->markerCount; i++)
		{
			markids[i] = track->markers[i].id;
		}
	}

	return track->markerCount;
}

AFframecount afGetMarkPosition (AFfilehandle file, int trackid, int markid)
{
	if (!_af_filehandle_ok(file))
		return 0L;

	Track *track = file->getTrack(trackid);
	if (!track)
		return 0L;

	Marker *marker = track->getMarker(markid);
	if (!marker)
		return 0L;

	return marker->position;
}

Marker *_af_marker_new (int count)
{
	Marker	*markers = (Marker *) _af_calloc(count, sizeof (Marker));
	if (markers == NULL)
		return NULL;

	return markers;
}
