noinst_LTLIBRARIES = libalac.la

libalac_la_SOURCES = \
	ag_dec.c \
	ag_enc.c \
	aglib.h \
	ALACAudioTypes.h \
	ALACBitUtilities.c \
	ALACBitUtilities.h \
	ALACDecoder.cpp \
	ALACDecoder.h \
	ALACEncoder.cpp \
	ALACEncoder.h \
	dp_dec.c \
	dp_enc.c \
	dplib.h \
	EndianPortable.c \
	EndianPortable.h \
	matrix_dec.c \
	matrix_enc.c \
	matrixlib.h

AM_CFLAGS = \
	-DNDEBUG \
	-Wall \
	-Wno-multichar \
	$(COVERAGE_CFLAGS) \
	-fvisibility=hidden
AM_CXXFLAGS = $(AM_CFLAGS) -fno-rtti -fno-exceptions -fvisibility-inlines-hidden

CLEANFILES = *.gcda *.gcno
