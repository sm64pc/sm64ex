/*
	Audio File Library

	Copyright 1998-2000, Michael Pruett <michael@68k.org>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#include <dmedia/audioutil.h>
#else
#include <audiofile.h>
#include <aupvlist.h>
#endif

#include <stdio.h>
#include <stdlib.h>

const char *paramtypename (int paramtype);
void printinstparams (int format);

#define DEBUG

#ifdef DEBUG
#define DEBG printf
#else
#define DEBG
#endif

int main (int ac, char **av)
{
	AUpvlist	formatlist;
	int		*flist;
	long		lvalue;
	int		i, formatcount;

	formatlist = afQuery(AF_QUERYTYPE_FILEFMT, AF_QUERY_IDS, 0, 0, 0);
	formatcount = afQueryLong(AF_QUERYTYPE_FILEFMT, AF_QUERY_ID_COUNT, 0, 0, 0);

	DEBG("formatcount = %d\n", formatcount);

	AUpvgetval(formatlist, 0, &flist);
	AUpvfree(formatlist);

	for (i=0; i<formatcount; i++)
	{
		int	format;
		char	*formatstring;

		format = flist[i];
		DEBG("format = %d\n", format);
		formatstring = afQueryPointer(AF_QUERYTYPE_FILEFMT, AF_QUERY_NAME,
			format, 0, 0);
		DEBG("format = %s\n", formatstring);

		lvalue = afQueryLong(AF_QUERYTYPE_INST, AF_QUERY_SUPPORTED,
			format, 0, 0);
		DEBG("instrument query: supported: %ld\n", lvalue);

		lvalue = afQueryLong(AF_QUERYTYPE_INST, AF_QUERY_MAX_NUMBER,
			format, 0, 0);
		DEBG("instrument query: maximum number: %ld\n", lvalue);

		lvalue = afQueryLong(AF_QUERYTYPE_INSTPARAM, AF_QUERY_SUPPORTED,
			format, 0, 0);
		DEBG("instrument parameter query: supported: %ld\n", lvalue);

		/*
			Print instrument parameter information only if
			instrument parameters are supported.
		*/
		if (lvalue)
			printinstparams(format);
	}
	free(flist);

	return 0;
}

void printinstparams (int format)
{
	int	i, *iarray;
	long	instParamCount;

	instParamCount = afQueryLong(AF_QUERYTYPE_INSTPARAM, AF_QUERY_ID_COUNT,
		format, 0, 0);
	DEBG("instrument parameter query: id count: %ld\n", instParamCount);

	iarray = afQueryPointer(AF_QUERYTYPE_INSTPARAM, AF_QUERY_IDS,
		format, 0, 0);

	if (iarray == NULL)
		printf("AF_QUERYTYPE_INSTPARAM failed for format %d\n", format);

	for (i=0; i<instParamCount; i++)
	{
		int		paramType;
		AUpvlist	defaultValue;

		DEBG("instrument parameter query: id: %d\n", iarray[i]);
		paramType = afQueryLong(AF_QUERYTYPE_INSTPARAM,
			AF_QUERY_TYPE, format, iarray[i], 0);

		DEBG("\ttype of parameter: %s\n", paramtypename(paramType));
		DEBG("\tname of parameter: %s\n",
			(char *) afQueryPointer(AF_QUERYTYPE_INSTPARAM,
				AF_QUERY_NAME, format, iarray[i], 0));

		defaultValue = afQuery(AF_QUERYTYPE_INSTPARAM, AF_QUERY_DEFAULT,
			format, iarray[i], 0);

		if (paramType == AU_PVTYPE_LONG)
		{
			long	ldefault;
			AUpvgetval(defaultValue, 0, &ldefault);
			DEBG("\tdefault value: %ld\n", ldefault);
		}
		else if (paramType == AU_PVTYPE_DOUBLE)
		{
			double	ddefault;
			AUpvgetval(defaultValue, 0, &ddefault);
			DEBG("\tdefault value: %f\n", ddefault);
		}
		else if (paramType == AU_PVTYPE_PTR)
		{
			void	*vdefault;
			AUpvgetval(defaultValue, 0, &vdefault);
			DEBG("\tdefault value: %p\n", vdefault);
		}

		AUpvfree(defaultValue);
	}

	free(iarray);
}

const char *paramtypename (int paramtype)
{
	static const char	*longname = "long";
	static const char	*doublename = "double";
	static const char	*pointername = "pointer";

	switch (paramtype)
	{
		case AU_PVTYPE_LONG:
			return longname;
		case AU_PVTYPE_DOUBLE:
			return doublename;
		case AU_PVTYPE_PTR:
			return pointername;
	}

	return NULL;
}
