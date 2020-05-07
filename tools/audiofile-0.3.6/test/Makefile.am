INCLUDES = -I$(top_srcdir)/libaudiofile -I$(top_srcdir)

TESTS = \
	ADPCM \
	AES \
	ALAC \
	ChannelMatrix \
	Error \
	FloatToInt \
	Instrument \
	IntToFloat \
	InvalidCompressionFormat \
	InvalidSampleFormat \
	Large \
	Loop \
	Marker \
	Miscellaneous \
	NeXT \
	PCMData \
	PCMMapping \
	Pipe \
	Query \
	SampleFormat \
	Seek \
	Sign \
	VirtualFile \
	floatto24 \
	query2 \
	sixteen-to-eight \
	testchannelmatrix \
	testdouble \
	testfloat \
	testmarkers \
	twentyfour \
	twentyfour2 \
	writealaw \
	writeraw \
	writeulaw

if ENABLE_FLAC
TESTS += FLAC
endif

check_PROGRAMS = \
	$(TESTS) \
	instparamtest \
	instparamwrite \
	printmarkers \
	testaupv

if ENABLE_VALGRIND
VALGRIND_FLAGS = --quiet --leak-check=full --show-reachable=yes --error-exitcode=1
TESTS_ENVIRONMENT = $(top_builddir)/libtool --mode=execute $(VALGRIND) $(VALGRIND_FLAGS)
endif

LIBAUDIOFILE = $(top_builddir)/libaudiofile/libaudiofile.la

LDADD = $(LIBAUDIOFILE)

DEPENDENCIES = $(LIBAUDIOFILE)

LIBGTEST = ../gtest/libgtest.la

ADPCM_SOURCES = ADPCM.cpp TestUtilities.cpp TestUtilities.h
ADPCM_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

AES_SOURCES = AES.cpp TestUtilities.cpp TestUtilities.h
AES_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

ALAC_SOURCES = ALAC.cpp Lossless.h TestUtilities.cpp TestUtilities.h
ALAC_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

ChannelMatrix_SOURCES = ChannelMatrix.cpp TestUtilities.cpp TestUtilities.h
ChannelMatrix_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Error_SOURCES = Error.cpp TestUtilities.cpp TestUtilities.h
Error_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

FLAC_SOURCES = FLAC.cpp Lossless.h TestUtilities.cpp TestUtilities.h
FLAC_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

FloatToInt_SOURCES = FloatToInt.cpp TestUtilities.cpp TestUtilities.h
FloatToInt_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Instrument_SOURCES = Instrument.cpp TestUtilities.cpp TestUtilities.h
Instrument_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

IntToFloat_SOURCES = IntToFloat.cpp TestUtilities.cpp TestUtilities.h
IntToFloat_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

InvalidCompressionFormat_SOURCES = InvalidCompressionFormat.cpp TestUtilities.cpp TestUtilities.h
InvalidCompressionFormat_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

InvalidSampleFormat_SOURCES = InvalidSampleFormat.cpp TestUtilities.cpp TestUtilities.h
InvalidSampleFormat_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Large_SOURCES = Large.cpp TestUtilities.cpp TestUtilities.h
Large_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Loop_SOURCES = Loop.cpp TestUtilities.cpp TestUtilities.h
Loop_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Marker_SOURCES = Marker.cpp TestUtilities.cpp TestUtilities.h
Marker_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Miscellaneous_SOURCES = Miscellaneous.cpp TestUtilities.cpp TestUtilities.h
Miscellaneous_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

NeXT_SOURCES = NeXT.cpp TestUtilities.cpp TestUtilities.h
NeXT_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

PCMData_SOURCES = PCMData.cpp TestUtilities.cpp TestUtilities.h
PCMData_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

PCMMapping_SOURCES = PCMMapping.cpp TestUtilities.cpp TestUtilities.h
PCMMapping_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Pipe_SOURCES = Pipe.cpp TestUtilities.cpp TestUtilities.h
Pipe_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Query_SOURCES = Query.cpp TestUtilities.cpp TestUtilities.h
Query_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

SampleFormat_SOURCES = SampleFormat.cpp TestUtilities.cpp TestUtilities.h
SampleFormat_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Seek_SOURCES = Seek.cpp TestUtilities.cpp TestUtilities.h
Seek_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

Sign_SOURCES = Sign.cpp TestUtilities.cpp TestUtilities.h
Sign_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

VirtualFile_SOURCES = VirtualFile.cpp TestUtilities.cpp TestUtilities.h
VirtualFile_LDADD = $(LIBGTEST) $(LIBAUDIOFILE)

floatto24_SOURCES = floatto24.c TestUtilities.cpp TestUtilities.h

printmarkers_SOURCES = printmarkers.c
printmarkers_LDADD = $(LIBAUDIOFILE) -lm

sixteen_to_eight_SOURCES = sixteen-to-eight.c TestUtilities.cpp TestUtilities.h

testchannelmatrix_SOURCES = testchannelmatrix.c TestUtilities.cpp TestUtilities.h

testdouble_SOURCES = testdouble.c TestUtilities.cpp TestUtilities.h

testfloat_SOURCES = testfloat.c TestUtilities.cpp TestUtilities.h

testmarkers_SOURCES = testmarkers.c TestUtilities.cpp TestUtilities.h

twentyfour_SOURCES = twentyfour.c TestUtilities.cpp TestUtilities.h

twentyfour2_SOURCES = twentyfour2.c TestUtilities.cpp TestUtilities.h

writealaw_SOURCES = writealaw.c TestUtilities.cpp TestUtilities.h

writeraw_SOURCES = writeraw.c TestUtilities.cpp TestUtilities.h

writeulaw_SOURCES = writeulaw.c TestUtilities.cpp TestUtilities.h

AM_CXXFLAGS = -fno-rtti -fno-exceptions -DGTEST_HAS_RTTI=0 -DGTEST_HAS_EXCEPTIONS=0
