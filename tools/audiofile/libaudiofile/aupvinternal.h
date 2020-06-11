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
	aupvinternal.h

	This file contains the private data structures for the parameter
	value list data types.
*/

#ifndef AUPVINTERNAL_H
#define AUPVINTERNAL_H

struct _AUpvitem
{
	int	valid;
	int	type;
	int	parameter;

	union
	{
		long	l;
		double	d;
		void	*v;
	}
	value;
};

struct _AUpvlist
{
	int			valid;
	size_t			count;
	struct _AUpvitem	*items;
};

enum
{
	_AU_VALID_PVLIST = 30932,
	_AU_VALID_PVITEM = 30933
};

enum
{
	AU_BAD_PVLIST = -5,
	AU_BAD_PVITEM = -6,
	AU_BAD_PVTYPE = -7,
	AU_BAD_ALLOC = -8
};

enum
{
	_AU_FAIL = -1,
	_AU_SUCCESS = 0
};

#define _AU_NULL_PVITEM ((struct _AUpvitem *) NULL)

#endif
