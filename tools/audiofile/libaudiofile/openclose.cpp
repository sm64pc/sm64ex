/*
	Audio File Library
	Copyright (C) 2000-2001, Silicon Graphics, Inc.

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

#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <audiofile.h>

#include "File.h"
#include "FileHandle.h"
#include "Instrument.h"
#include "Marker.h"
#include "Setup.h"
#include "Track.h"
#include "afinternal.h"
#include "modules/Module.h"
#include "modules/ModuleState.h"
#include "units.h"
#include "util.h"

static status _afOpenFile (int access, File *f, const char *filename,
	AFfilehandle *file, AFfilesetup filesetup);

int _af_identify (File *f, int *implemented)
{
	if (!f->canSeek())
	{
		_af_error(AF_BAD_LSEEK, "Cannot seek in file");
		return AF_FILE_UNKNOWN;
	}

	AFfileoffset curpos = f->tell();

	for (int i=0; i<_AF_NUM_UNITS; i++)
	{
		if (_af_units[i].recognize &&
			_af_units[i].recognize(f))
		{
			if (implemented != NULL)
				*implemented = _af_units[i].implemented;
			f->seek(curpos, File::SeekFromBeginning);
			return _af_units[i].fileFormat;
		}
	}

	f->seek(curpos, File::SeekFromBeginning);

	if (implemented != NULL)
		*implemented = false;

	return AF_FILE_UNKNOWN;
}

int afIdentifyFD (int fd)
{
	/*
		Duplicate the file descriptor since otherwise the
		original file descriptor would get closed when we close
		the virtual file below.
	*/
	fd = dup(fd);
	File *f = File::create(fd, File::ReadAccess);

	int result = _af_identify(f, NULL);

	delete f;

	return result;
}

int afIdentifyNamedFD (int fd, const char *filename, int *implemented)
{
	/*
		Duplicate the file descriptor since otherwise the
		original file descriptor would get closed when we close
		the virtual file below.
	*/
	fd = dup(fd);

	File *f = File::create(fd, File::ReadAccess);
	if (!f)
	{
		_af_error(AF_BAD_OPEN, "could not open file '%s'", filename);
		return AF_FILE_UNKNOWN;
	}

	int result = _af_identify(f, implemented);

	delete f;

	return result;
}

AFfilehandle afOpenFD (int fd, const char *mode, AFfilesetup setup)
{
	if (!mode)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	int access;
	if (mode[0] == 'r')
	{
		access = _AF_READ_ACCESS;
	}
	else if (mode[0] == 'w')
	{
		access = _AF_WRITE_ACCESS;
	}
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	File *f = File::create(fd, access == _AF_READ_ACCESS ?
		File::ReadAccess : File::WriteAccess);

	AFfilehandle filehandle = NULL;
	if (_afOpenFile(access, f, NULL, &filehandle, setup) != AF_SUCCEED)
	{
		delete f;
	}

	return filehandle;
}

AFfilehandle afOpenNamedFD (int fd, const char *mode, AFfilesetup setup,
	const char *filename)
{
	if (!mode)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	int access;
	if (mode[0] == 'r')
		access = _AF_READ_ACCESS;
	else if (mode[0] == 'w')
		access = _AF_WRITE_ACCESS;
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	File *f = File::create(fd, access == _AF_READ_ACCESS ?
		File::ReadAccess : File::WriteAccess);

	AFfilehandle filehandle;
	if (_afOpenFile(access, f, filename, &filehandle, setup) != AF_SUCCEED)
	{
		delete f;
	}

	return filehandle;
}

AFfilehandle afOpenFile (const char *filename, const char *mode, AFfilesetup setup)
{
	if (!mode)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	int access;
	if (mode[0] == 'r')
	{
		access = _AF_READ_ACCESS;
	}
	else if (mode[0] == 'w')
	{
		access = _AF_WRITE_ACCESS;
	}
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	File *f = File::open(filename,
		access == _AF_READ_ACCESS ? File::ReadAccess : File::WriteAccess);
	if (!f)
	{
		_af_error(AF_BAD_OPEN, "could not open file '%s'", filename);
		return AF_NULL_FILEHANDLE;
	}

	AFfilehandle filehandle;
	if (_afOpenFile(access, f, filename, &filehandle, setup) != AF_SUCCEED)
	{
		delete f;
	}

	return filehandle;
}

AFfilehandle afOpenVirtualFile (AFvirtualfile *vf, const char *mode,
	AFfilesetup setup)
{
	if (!vf)
	{
		_af_error(AF_BAD_OPEN, "null virtual file");
		return AF_NULL_FILEHANDLE;
	}

	if (!mode)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	int access;
	if (mode[0] == 'r')
	{
		access = _AF_READ_ACCESS;
	}
	else if (mode[0] == 'w')
	{
		access = _AF_WRITE_ACCESS;
	}
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	File *f = File::create(vf,
		access == _AF_READ_ACCESS ? File::ReadAccess : File::WriteAccess);
	if (!f)
	{
		_af_error(AF_BAD_OPEN, "could not open virtual file");
		return AF_NULL_FILEHANDLE;
	}

	AFfilehandle filehandle;
	if (_afOpenFile(access, f, NULL, &filehandle, setup) != AF_SUCCEED)
	{
		delete f;
	}

	return filehandle;
}

static status _afOpenFile (int access, File *f, const char *filename,
	AFfilehandle *file, AFfilesetup filesetup)
{
	int	fileFormat = AF_FILE_UNKNOWN;
	int	implemented = true;

	int		userSampleFormat = 0;
	double		userSampleRate = 0.0;
	PCMInfo	userPCM = {0};
	bool		userFormatSet = false;

	AFfilehandle	filehandle = AF_NULL_FILEHANDLE;
	AFfilesetup	completesetup = AF_NULL_FILESETUP;

	*file = AF_NULL_FILEHANDLE;

	if (access == _AF_WRITE_ACCESS || filesetup != AF_NULL_FILESETUP)
	{
		if (!_af_filesetup_ok(filesetup))
			return AF_FAIL;

		fileFormat = filesetup->fileFormat;
		if (access == _AF_READ_ACCESS && fileFormat != AF_FILE_RAWDATA)
		{
			_af_error(AF_BAD_FILESETUP,
				"warning: opening file for read access: "
				"ignoring file setup with non-raw file format");
			filesetup = AF_NULL_FILESETUP;
			fileFormat = _af_identify(f, &implemented);
		}
	}
	else if (filesetup == AF_NULL_FILESETUP)
		fileFormat = _af_identify(f, &implemented);

	if (fileFormat == AF_FILE_UNKNOWN)
	{
		if (filename != NULL)
			_af_error(AF_BAD_NOT_IMPLEMENTED,
				"'%s': unrecognized audio file format",
				filename);
		else
			_af_error(AF_BAD_NOT_IMPLEMENTED,
				"unrecognized audio file format");
		return AF_FAIL;
	}

	const char *formatName = _af_units[fileFormat].name;

	if (!implemented)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED,
			"%s format not currently supported", formatName);
	}

	completesetup = NULL;
	if (filesetup != AF_NULL_FILESETUP)
	{
		userSampleFormat = filesetup->tracks[0].f.sampleFormat;
		userPCM = filesetup->tracks[0].f.pcm;
		userSampleRate = filesetup->tracks[0].f.sampleRate;
		userFormatSet = true;
		if ((completesetup = _af_units[fileFormat].completesetup(filesetup)) == NULL)
			return AF_FAIL;
	}

	filehandle = _AFfilehandle::create(fileFormat);
	if (!filehandle)
	{
		if (completesetup)
			afFreeFileSetup(completesetup);
		return AF_FAIL;
	}

	filehandle->m_fh = f;
	filehandle->m_access = access;
	filehandle->m_seekok = f->canSeek();
	if (filename != NULL)
		filehandle->m_fileName = strdup(filename);
	else
		filehandle->m_fileName = NULL;
	filehandle->m_fileFormat = fileFormat;

	status result = access == _AF_READ_ACCESS ?
		filehandle->readInit(completesetup) :
		filehandle->writeInit(completesetup);

	if (result != AF_SUCCEED)
	{
		delete filehandle;
		filehandle = AF_NULL_FILEHANDLE;
		if (completesetup)
			afFreeFileSetup(completesetup);
		return AF_FAIL;
	}

	if (completesetup)
		afFreeFileSetup(completesetup);

	/*
		Initialize virtual format.
	*/
	for (int t=0; t<filehandle->m_trackCount; t++)
	{
		Track *track = &filehandle->m_tracks[t];

		track->v = track->f;

		if (userFormatSet)
		{
			track->v.sampleFormat = userSampleFormat;
			track->v.pcm = userPCM;
			track->v.sampleRate = userSampleRate;
		}

		track->v.compressionType = AF_COMPRESSION_NONE;
		track->v.compressionParams = NULL;

#if WORDS_BIGENDIAN
		track->v.byteOrder = AF_BYTEORDER_BIGENDIAN;
#else
		track->v.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
#endif

		track->ms = new ModuleState();
		if (track->ms->init(filehandle, track) == AF_FAIL)
		{
			delete filehandle;
			return AF_FAIL;
		}
	}

	*file = filehandle;

	return AF_SUCCEED;
}

int afSyncFile (AFfilehandle handle)
{
	if (!_af_filehandle_ok(handle))
		return -1;

	if (handle->m_access == _AF_WRITE_ACCESS)
	{
		/* Finish writes on all tracks. */
		for (int trackno = 0; trackno < handle->m_trackCount; trackno++)
		{
			Track *track = &handle->m_tracks[trackno];

			if (track->ms->isDirty() && track->ms->setup(handle, track) == AF_FAIL)
				return -1;

			if (track->ms->sync(handle, track) != AF_SUCCEED)
				return -1;
		}

		/* Update file headers. */
		if (handle->update() != AF_SUCCEED)
			return AF_FAIL;
	}
	else if (handle->m_access == _AF_READ_ACCESS)
	{
		/* Do nothing. */
	}
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode %d",
			handle->m_access);
		return AF_FAIL;
	}

	return AF_SUCCEED;
}

int afCloseFile (AFfilehandle file)
{
	int	err;

	if (!_af_filehandle_ok(file))
		return -1;

	afSyncFile(file);

	err = file->m_fh->close();
	if (err < 0)
		_af_error(AF_BAD_CLOSE, "close returned %d", err);

	delete file->m_fh;
	delete file;

	return 0;
}
