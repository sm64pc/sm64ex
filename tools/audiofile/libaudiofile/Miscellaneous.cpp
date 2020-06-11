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
	Miscellaneous.cpp

	This file contains routines for dealing with the Audio File
	Library's internal miscellaneous data types.
*/

#include "config.h"

#include <algorithm>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "FileHandle.h"
#include "Setup.h"
#include "afinternal.h"
#include "audiofile.h"
#include "util.h"

void afInitMiscIDs (AFfilesetup setup, const int *ids, int nids)
{
	if (!_af_filesetup_ok(setup))
		return;

	if (setup->miscellaneous != NULL)
	{
		free(setup->miscellaneous);
	}

	setup->miscellaneousCount = nids;

	if (nids == 0)
		setup->miscellaneous = NULL;
	else
	{
		setup->miscellaneous = (MiscellaneousSetup *) _af_calloc(nids,
			sizeof (MiscellaneousSetup));

		if (setup->miscellaneous == NULL)
			return;

		for (int i=0; i<nids; i++)
		{
			setup->miscellaneous[i].id = ids[i];
			setup->miscellaneous[i].type = 0;
			setup->miscellaneous[i].size = 0;
		}
	}

	setup->miscellaneousSet = true;
}

int afGetMiscIDs (AFfilehandle file, int *ids)
{
	if (!_af_filehandle_ok(file))
		return -1;

	if (ids != NULL)
	{
		for (int i=0; i<file->m_miscellaneousCount; i++)
		{
			ids[i] = file->m_miscellaneous[i].id;
		}
	}

	return file->m_miscellaneousCount;
}

void afInitMiscType (AFfilesetup setup, int miscellaneousid, int type)
{
	if (!_af_filesetup_ok(setup))
		return;

	MiscellaneousSetup *miscellaneous = setup->getMiscellaneous(miscellaneousid);
	if (miscellaneous)
		miscellaneous->type = type;
}

int afGetMiscType (AFfilehandle file, int miscellaneousid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Miscellaneous *miscellaneous = file->getMiscellaneous(miscellaneousid);
	if (miscellaneous)
		return miscellaneous->type;
	return -1;
}

void afInitMiscSize (AFfilesetup setup, int miscellaneousid, int size)
{
	if (!_af_filesetup_ok(setup))
		return;

	MiscellaneousSetup *miscellaneous = setup->getMiscellaneous(miscellaneousid);
	if (miscellaneous)
		miscellaneous->size = size;
}

int afGetMiscSize (AFfilehandle file, int miscellaneousid)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Miscellaneous *miscellaneous = file->getMiscellaneous(miscellaneousid);
	if (miscellaneous)
		return miscellaneous->size;
	return -1;
}

int afWriteMisc (AFfilehandle file, int miscellaneousid, const void *buf, int bytes)
{
	if (!_af_filehandle_ok(file))
		return -1;

	if (!file->checkCanWrite())
		return -1;

	Miscellaneous *miscellaneous = file->getMiscellaneous(miscellaneousid);
	if (!miscellaneous)
		return -1;

	if (bytes <= 0)
	{
		_af_error(AF_BAD_MISCSIZE, "invalid size (%d) for miscellaneous chunk", bytes);
		return -1;
	}

	if (miscellaneous->buffer == NULL && miscellaneous->size != 0)
	{
		miscellaneous->buffer = _af_malloc(miscellaneous->size);
		if (miscellaneous->buffer == NULL)
			return -1;
		memset(miscellaneous->buffer, 0, miscellaneous->size);
	}

	int localsize = std::min(bytes,
		miscellaneous->size - miscellaneous->position);
	memcpy((char *) miscellaneous->buffer + miscellaneous->position,
		buf, localsize);
	miscellaneous->position += localsize;
	return localsize;
}

int afReadMisc (AFfilehandle file, int miscellaneousid, void *buf, int bytes)
{
	if (!_af_filehandle_ok(file))
		return -1;

	if (!file->checkCanRead())
		return -1;

	Miscellaneous *miscellaneous = file->getMiscellaneous(miscellaneousid);
	if (!miscellaneous)
		return -1;

	if (bytes <= 0)
	{
		_af_error(AF_BAD_MISCSIZE, "invalid size (%d) for miscellaneous chunk", bytes);
		return -1;
	}

	int localsize = std::min(bytes,
		miscellaneous->size - miscellaneous->position);
	memcpy(buf, (char *) miscellaneous->buffer + miscellaneous->position,
		localsize);
	miscellaneous->position += localsize;
	return localsize;
}

int afSeekMisc (AFfilehandle file, int miscellaneousid, int offset)
{
	if (!_af_filehandle_ok(file))
		return -1;

	Miscellaneous *miscellaneous = file->getMiscellaneous(miscellaneousid);
	if (!miscellaneous)
		return -1;

	if (offset >= miscellaneous->size)
	{
		_af_error(AF_BAD_MISCSEEK,
			"offset %d too big for miscellaneous chunk %d "
			"(%d data bytes)",
			offset, miscellaneousid, miscellaneous->size);
		return -1;
	}

	miscellaneous->position = offset;

	return offset;
}
