UNAME := $(shell uname)
DEBUG ?= 0

ifeq ($(UNAME),Darwin)
  OSX_BUILD := -DOSX_BUILD
endif

ifeq ($(DEBUG),1)
  OPT_FLAG := -g
else
  OPT_FLAG := -O2
endif

CC := gcc
CXX := g++
CFLAGS := -Llib -Iinclude -I../include -I . -Wall -Wextra -Wno-unused-parameter $(OSX_BUILD) -pedantic -std=c99 $(OPT_FLAG) -s
PROGRAMS := n64graphics n64graphics_ci mio0 n64cksum textconv patch_libultra_math iplfontutil aifc_decode aiff_extract_codebook vadpcm_enc tabledesign extract_data_for_mio skyconv

n64graphics_SOURCES := n64graphics.c utils.c
n64graphics_CFLAGS  := -DN64GRAPHICS_STANDALONE

n64graphics_ci_SOURCES := n64graphics_ci_dir/n64graphics_ci.c n64graphics_ci_dir/exoquant/exoquant.c n64graphics_ci_dir/utils.c
n64graphics_ci_CFLAGS := $(OPT_FLAG)

mio0_SOURCES := libmio0.c
mio0_CFLAGS := -DMIO0_STANDALONE

n64cksum_SOURCES := n64cksum.c libmio0.c libsm64.c utils.c

textconv_SOURCES := textconv.c utf8.c hashtable.c
patch_libultra_math_SOURCES := patch_libultra_math.c

iplfontutil_SOURCES := iplfontutil.c
iplfontutil_CFLAGS := $(OPT_FLAG)

aifc_decode_SOURCES := aifc_decode.c
aifc_decode_CFLAGS := $(OPT_FLAG)

aiff_extract_codebook_SOURCES := aiff_extract_codebook.c

tabledesign_SOURCES := sdk-tools/tabledesign/codebook.c sdk-tools/tabledesign/estimate.c sdk-tools/tabledesign/print.c sdk-tools/tabledesign/tabledesign.c
tabledesign_CFLAGS := -Wno-uninitialized -laudiofile -lstdc++

vadpcm_enc_SOURCES := sdk-tools/adpcm/vadpcm_enc.c sdk-tools/adpcm/vpredictor.c sdk-tools/adpcm/quant.c sdk-tools/adpcm/util.c sdk-tools/adpcm/vencode.c
vadpcm_enc_CFLAGS := -Wno-unused-result -Wno-uninitialized -Wno-sign-compare -Wno-absolute-value

extract_data_for_mio_SOURCES := extract_data_for_mio.c
extract_data_for_mio_CFLAGS := $(OPT_FLAG)

skyconv_SOURCES := skyconv.c n64graphics.c utils.c
skyconv_CFLAGS := $(OPT_FLAG) -lm

all: $(PROGRAMS)

clean:
	$(RM) $(PROGRAMS)
	$(RM) gen_asset_list

define COMPILE
$(1): $($1_SOURCES)
	$(CC) $(CFLAGS) $(OSX_BUILD) $$^ -lm -o $$@ $($1_CFLAGS)
endef

# Separate build for debugging gen_asset_list.cpp
gen_asset_list:
	$(CXX) -std=c++17 gen_asset_list.cpp -lstdc++fs $(OPT_FLAG) -Wall -o gen_asset_list

$(foreach p,$(PROGRAMS),$(eval $(call COMPILE,$(p))))