SUBDIRS = alac modules

lib_LTLIBRARIES = libaudiofile.la

EXTRA_DIST = audiofile.exports

libaudiofile_la_SOURCES = \
	AIFF.cpp \
	AIFF.h \
	AVR.cpp \
	AVR.h \
	AudioFormat.cpp \
	AudioFormat.h \
	Buffer.cpp \
	Buffer.h \
	CAF.cpp \
	CAF.h \
	Compiler.h \
	FLACFile.cpp \
	FLACFile.h \
	Features.h \
	File.cpp \
	File.h \
	FileHandle.cpp \
	FileHandle.h \
	IFF.cpp \
	IFF.h \
	IRCAM.cpp \
	IRCAM.h \
	Instrument.cpp \
	Instrument.h \
	Loop.cpp \
	Marker.cpp \
	Marker.h \
	Miscellaneous.cpp \
	NIST.cpp \
	NIST.h \
	NeXT.cpp \
	NeXT.h \
	PacketTable.cpp \
	PacketTable.h \
	Raw.cpp \
	Raw.h \
	SampleVision.cpp \
	SampleVision.h \
	Setup.cpp \
	Setup.h \
	Shared.h \
	Tag.h \
	Track.cpp \
	Track.h \
	UUID.cpp \
	UUID.h \
	VOC.cpp \
	VOC.h \
	WAVE.cpp \
	WAVE.h \
	aes.cpp \
	af_vfs.cpp \
	af_vfs.h \
	afinternal.h \
	aupv.c \
	aupvinternal.h \
	aupvlist.h \
	byteorder.h \
	byteorder.h \
	compression.cpp \
	compression.h \
	data.cpp \
	debug.cpp \
	debug.h \
	error.c \
	error.h \
	extended.c \
	extended.h \
	format.cpp \
	g711.c \
	g711.h \
	openclose.cpp \
	pcm.cpp \
	pcm.h \
	query.cpp \
	units.cpp \
	units.h \
	util.cpp \
	util.h \
	util.h

libaudiofile_la_LIBADD = \
	modules/libmodules.la \
	alac/libalac.la \
	$(COVERAGE_LIBS) \
	$(FLAC_LIBS) \
	-lm

libaudiofile_la_LDFLAGS = -version-info $(AUDIOFILE_VERSION_INFO) \
	-no-undefined \
	-export-symbols $(srcdir)/audiofile.exports

include_HEADERS = audiofile.h aupvlist.h af_vfs.h

TESTS = UnitTests

check_PROGRAMS = $(TESTS)

if ENABLE_VALGRIND
VALGRIND_FLAGS = --quiet --leak-check=full --show-reachable=yes --error-exitcode=1
TESTS_ENVIRONMENT = $(top_builddir)/libtool --mode=execute $(VALGRIND) $(VALGRIND_FLAGS)
endif

LIBGTEST = ../gtest/libgtest.la

UnitTests_SOURCES = modules/UT_RebufferModule.cpp
UnitTests_LDADD = libaudiofile.la $(LIBGTEST)
UnitTests_CPPFLAGS = -I$(top_srcdir)
UnitTests_CXXFLAGS = -fno-rtti -fno-exceptions -DGTEST_HAS_RTTI=0 -DGTEST_HAS_EXCEPTIONS=0
UnitTests_LDFLAGS = -static

# GNU gcc
# AM_CFLAGS = -Wall -g
# SGI MIPSpro cc
# AM_CFLAGS = -fullwarn -g
# No debugging.
AM_CFLAGS = -DNDEBUG -Wall $(WERROR_CFLAGS) $(COVERAGE_CFLAGS) $(FLAC_CFLAGS) -fvisibility=hidden
AM_CXXFLAGS = $(AM_CFLAGS) -fno-rtti -fno-exceptions -fvisibility-inlines-hidden

CLEANFILES = *.gcda *.gcno
