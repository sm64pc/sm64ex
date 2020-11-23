CC ?= gcc
CXX ?= g++
CFLAGS := -I../include -I. -Wall -Wextra -Wno-unused-parameter -pedantic -std=c99 -O2 -s
LDFLAGS := -lm
PROGRAMS := n64graphics aifc_decode aiff_extract_codebook mio0 vadpcm_enc tabledesign skyconv

default: all

mio0_SOURCES := libmio0.c
mio0_CFLAGS := -DMIO0_STANDALONE

n64graphics_SOURCES := n64graphics.c utils.c
n64graphics_CFLAGS  := -DN64GRAPHICS_STANDALONE

aifc_decode_SOURCES := aifc_decode.c

aiff_extract_codebook_SOURCES := aiff_extract_codebook.c

extract_data_for_mio_SOURCES := extract_data_for_mio.c

skyconv_SOURCES := skyconv.c n64graphics.c utils.c

tabledesign_SOURCES := sdk-tools/tabledesign/codebook.c sdk-tools/tabledesign/estimate.c sdk-tools/tabledesign/print.c sdk-tools/tabledesign/tabledesign.c
tabledesign_CFLAGS := -Iaudiofile -Wno-uninitialized
tabledesign_LDFLAGS := -Laudiofile -laudiofile -lstdc++ -lm

vadpcm_enc_SOURCES := sdk-tools/adpcm/vadpcm_enc.c sdk-tools/adpcm/vpredictor.c sdk-tools/adpcm/quant.c sdk-tools/adpcm/util.c sdk-tools/adpcm/vencode.c
vadpcm_enc_CFLAGS := -Wno-unused-result -Wno-uninitialized -Wno-sign-compare -Wno-absolute-value


LIBAUDIOFILE := audiofile/libaudiofile.a

$(LIBAUDIOFILE):
	@CC=$(CC) CXX=$(CXX) $(MAKE) -C audiofile

all: $(LIBAUDIOFILE) $(PROGRAMS) $(CXX_PROGRAMS)

clean:
	$(RM) $(PROGRAMS) $(CXX_PROGRAMS)
	$(MAKE) -C audiofile clean

define COMPILE
$(1): $($1_SOURCES)
	$(CC) $(CFLAGS) $($1_CFLAGS) $$^ -o $$@ $(LDFLAGS) $($1_LDFLAGS)
endef

$(foreach p,$(PROGRAMS),$(eval $(call COMPILE,$(p))))

.PHONY: all clean default
