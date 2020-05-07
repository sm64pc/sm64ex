INCLUDES = -I$(top_srcdir)/libaudiofile

bin_PROGRAMS = sfconvert sfinfo

sfconvert_SOURCES = sfconvert.c printinfo.c printinfo.h

sfinfo_SOURCES = sfinfo.c printinfo.c printinfo.h

LDADD = $(top_builddir)/libaudiofile/libaudiofile.la

DEPENDENCIES = $(top_builddir)/libaudiofile/libaudiofile.la

AM_CFLAGS = -Wall $(WERROR_CFLAGS)
