NATIVE_CC := gcc
NATIVE_CFLAGS := -g -Wall -O2 -Wno-unused-result -Wno-uninitialized

default: native
all: native

native: vadpcm_dec_native vadpcm_enc_native

clean:
	$(RM) *.o vadpcm_dec_native vadpcm_enc_native

vadpcm_dec_native: vadpcm_dec.c vpredictor.c sampleio.c vdecode.c util.c
	$(NATIVE_CC) $(NATIVE_CFLAGS) $^ -o $@ -lm

vadpcm_enc_native: vadpcm_enc.c vpredictor.c quant.c util.c vencode.c
	$(NATIVE_CC) $(NATIVE_CFLAGS) $^ -o $@ -lm

.PHONY: default all native clean
