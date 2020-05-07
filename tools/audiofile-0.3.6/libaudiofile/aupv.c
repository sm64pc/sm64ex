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
	aupv.c

	This file contains an implementation of SGI's Audio Library parameter
	value list functions.
*/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "aupvinternal.h"
#include "aupvlist.h"

AUpvlist AUpvnew (int maxitems)
{
	AUpvlist	aupvlist;
	int		i;

	if (maxitems <= 0)
		return AU_NULL_PVLIST;

	aupvlist = (AUpvlist) malloc(sizeof (struct _AUpvlist));
	assert(aupvlist);
	if (aupvlist == NULL)
		return AU_NULL_PVLIST;

	aupvlist->items = calloc(maxitems, sizeof (struct _AUpvitem));

	assert(aupvlist->items);
	if (aupvlist->items == NULL)
	{
		free(aupvlist);
		return AU_NULL_PVLIST;
	}

	/* Initialize the items in the list. */
	for (i=0; i<maxitems; i++)
	{
		aupvlist->items[i].valid = _AU_VALID_PVITEM;
		aupvlist->items[i].type = AU_PVTYPE_LONG;
		aupvlist->items[i].parameter = 0;
		memset(&aupvlist->items[i].value, 0, sizeof (aupvlist->items[i].value));
	}

	aupvlist->valid = _AU_VALID_PVLIST;
	aupvlist->count = maxitems;

	return aupvlist;
}

int AUpvgetmaxitems (AUpvlist list)
{
	assert(list);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;

	return list->count;
}

int AUpvfree (AUpvlist list)
{
	assert(list);
	assert(list->items);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;

	if ((list->items != _AU_NULL_PVITEM) &&
		(list->items[0].valid == _AU_VALID_PVITEM))
	{
		free(list->items);
	}

	free(list);

	return _AU_SUCCESS;
}

int AUpvsetparam (AUpvlist list, int item, int param)
{
	assert(list);
	assert(list->items);
	assert(item >= 0);
	assert(item < list->count);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;
	if ((item < 0) || (item > list->count - 1))
		return AU_BAD_PVITEM;
	if (list->items[item].valid != _AU_VALID_PVITEM)
		return AU_BAD_PVLIST;

	list->items[item].parameter = param;
	return _AU_SUCCESS;
}

int AUpvsetvaltype (AUpvlist list, int item, int type)
{
	assert(list);
	assert(list->items);
	assert(item >= 0);
	assert(item < list->count);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;
	if ((item < 0) || (item > list->count - 1))
		return AU_BAD_PVITEM;
	if (list->items[item].valid != _AU_VALID_PVITEM)
		return AU_BAD_PVLIST;

	list->items[item].type = type;
	return _AU_SUCCESS;
}

int AUpvsetval (AUpvlist list, int item, void *val)
{
	assert(list);
	assert(list->items);
	assert(item >= 0);
	assert(item < list->count);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;
	if ((item < 0) || (item > list->count - 1))
		return AU_BAD_PVITEM;
	if (list->items[item].valid != _AU_VALID_PVITEM)
		return AU_BAD_PVLIST;

	switch (list->items[item].type)
	{
		case AU_PVTYPE_LONG:
			list->items[item].value.l = *((long *) val);
			break;
		case AU_PVTYPE_DOUBLE:
			list->items[item].value.d = *((double *) val);
			break;
		case AU_PVTYPE_PTR:
			list->items[item].value.v = *((void **) val);
			break;
		default:
			assert(0);
			return AU_BAD_PVLIST;
	}

	return _AU_SUCCESS;
}

int AUpvgetparam (AUpvlist list, int item, int *param)
{
	assert(list);
	assert(list->items);
	assert(item >= 0);
	assert(item < list->count);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;
	if ((item < 0) || (item > list->count - 1))
		return AU_BAD_PVITEM;
	if (list->items[item].valid != _AU_VALID_PVITEM)
		return AU_BAD_PVLIST;

	*param = list->items[item].parameter;
	return _AU_SUCCESS;
}

int AUpvgetvaltype (AUpvlist list, int item, int *type)
{
	assert(list);
	assert(list->items);
	assert(item >= 0);
	assert(item < list->count);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;
	if ((item < 0) || (item > list->count - 1))
		return AU_BAD_PVITEM;
	if (list->items[item].valid != _AU_VALID_PVITEM)
		return AU_BAD_PVLIST;

	*type = list->items[item].type;
	return _AU_SUCCESS;
}

int AUpvgetval (AUpvlist list, int item, void *val)
{
	assert(list);
	assert(list->items);
	assert(item >= 0);
	assert(item < list->count);

	if (list == AU_NULL_PVLIST)
		return AU_BAD_PVLIST;
	if (list->valid != _AU_VALID_PVLIST)
		return AU_BAD_PVLIST;
	if ((item < 0) || (item > list->count - 1))
		return AU_BAD_PVITEM;
	if (list->items[item].valid != _AU_VALID_PVITEM)
		return AU_BAD_PVLIST;

	switch (list->items[item].type)
	{
		case AU_PVTYPE_LONG:
			*((long *) val) = list->items[item].value.l;
			break;
		case AU_PVTYPE_DOUBLE:
			*((double *) val) = list->items[item].value.d;
			break;
		case AU_PVTYPE_PTR:
			*((void **) val) = list->items[item].value.v;
			break;
	}

	return _AU_SUCCESS;
}
