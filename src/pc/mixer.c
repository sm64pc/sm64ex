#include <string.h>
#include <stdint.h>
#include <assert.h>

// Note: Some of this is stolen from Mupen64Plus rsp audio plugin.
// See abi.h for documentation.

#define DMEM_BASE 0x5c0

#define A_INIT			0x01
#define A_CONTINUE		0x00
#define A_LOOP                  0x02
#define A_OUT                   0x02
#define A_LEFT			0x02
#define	A_RIGHT			0x00
#define A_VOL			0x04
#define A_RATE			0x00
#define A_AUX			0x08
#define A_NOAUX			0x00
#define A_MAIN			0x00
#define A_MIX			0x10

struct alist_audio_t {
    /* main buffers */
    uint16_t in;
    uint16_t out;
    uint16_t count;

    /* auxiliary buffers */
    uint16_t dry_right;
    uint16_t wet_left;
    uint16_t wet_right;

    /* gains */
    int16_t dry;
    int16_t wet;

    /* envelopes (0:left, 1:right) */
    int16_t vol[2];
    int16_t target[2];
    int32_t rate[2];

    /* ADPCM loop point address */
    uint16_t *loop;

    /* storage for ADPCM table and polef coefficients */
    int16_t table[16 * 8];
};

struct ramp_t
{
    int64_t value;
    int64_t step;
    int64_t target;
};

struct env_mix_save_buffer_t {
    int16_t wet, pad0, dry, pad1;
    uint32_t ramp_targets[2];
    uint32_t exp_rates[2];
    uint32_t exp_seq[2];
    uint32_t ramp_values[2];
};

static const int16_t RESAMPLE_LUT[64 * 4] = {
    (int16_t)0x0c39, (int16_t)0x66ad, (int16_t)0x0d46, (int16_t)0xffdf,
    (int16_t)0x0b39, (int16_t)0x6696, (int16_t)0x0e5f, (int16_t)0xffd8,
    (int16_t)0x0a44, (int16_t)0x6669, (int16_t)0x0f83, (int16_t)0xffd0,
    (int16_t)0x095a, (int16_t)0x6626, (int16_t)0x10b4, (int16_t)0xffc8,
    (int16_t)0x087d, (int16_t)0x65cd, (int16_t)0x11f0, (int16_t)0xffbf,
    (int16_t)0x07ab, (int16_t)0x655e, (int16_t)0x1338, (int16_t)0xffb6,
    (int16_t)0x06e4, (int16_t)0x64d9, (int16_t)0x148c, (int16_t)0xffac,
    (int16_t)0x0628, (int16_t)0x643f, (int16_t)0x15eb, (int16_t)0xffa1,
    (int16_t)0x0577, (int16_t)0x638f, (int16_t)0x1756, (int16_t)0xff96,
    (int16_t)0x04d1, (int16_t)0x62cb, (int16_t)0x18cb, (int16_t)0xff8a,
    (int16_t)0x0435, (int16_t)0x61f3, (int16_t)0x1a4c, (int16_t)0xff7e,
    (int16_t)0x03a4, (int16_t)0x6106, (int16_t)0x1bd7, (int16_t)0xff71,
    (int16_t)0x031c, (int16_t)0x6007, (int16_t)0x1d6c, (int16_t)0xff64,
    (int16_t)0x029f, (int16_t)0x5ef5, (int16_t)0x1f0b, (int16_t)0xff56,
    (int16_t)0x022a, (int16_t)0x5dd0, (int16_t)0x20b3, (int16_t)0xff48,
    (int16_t)0x01be, (int16_t)0x5c9a, (int16_t)0x2264, (int16_t)0xff3a,
    (int16_t)0x015b, (int16_t)0x5b53, (int16_t)0x241e, (int16_t)0xff2c,
    (int16_t)0x0101, (int16_t)0x59fc, (int16_t)0x25e0, (int16_t)0xff1e,
    (int16_t)0x00ae, (int16_t)0x5896, (int16_t)0x27a9, (int16_t)0xff10,
    (int16_t)0x0063, (int16_t)0x5720, (int16_t)0x297a, (int16_t)0xff02,
    (int16_t)0x001f, (int16_t)0x559d, (int16_t)0x2b50, (int16_t)0xfef4,
    (int16_t)0xffe2, (int16_t)0x540d, (int16_t)0x2d2c, (int16_t)0xfee8,
    (int16_t)0xffac, (int16_t)0x5270, (int16_t)0x2f0d, (int16_t)0xfedb,
    (int16_t)0xff7c, (int16_t)0x50c7, (int16_t)0x30f3, (int16_t)0xfed0,
    (int16_t)0xff53, (int16_t)0x4f14, (int16_t)0x32dc, (int16_t)0xfec6,
    (int16_t)0xff2e, (int16_t)0x4d57, (int16_t)0x34c8, (int16_t)0xfebd,
    (int16_t)0xff0f, (int16_t)0x4b91, (int16_t)0x36b6, (int16_t)0xfeb6,
    (int16_t)0xfef5, (int16_t)0x49c2, (int16_t)0x38a5, (int16_t)0xfeb0,
    (int16_t)0xfedf, (int16_t)0x47ed, (int16_t)0x3a95, (int16_t)0xfeac,
    (int16_t)0xfece, (int16_t)0x4611, (int16_t)0x3c85, (int16_t)0xfeab,
    (int16_t)0xfec0, (int16_t)0x4430, (int16_t)0x3e74, (int16_t)0xfeac,
    (int16_t)0xfeb6, (int16_t)0x424a, (int16_t)0x4060, (int16_t)0xfeaf,
    (int16_t)0xfeaf, (int16_t)0x4060, (int16_t)0x424a, (int16_t)0xfeb6,
    (int16_t)0xfeac, (int16_t)0x3e74, (int16_t)0x4430, (int16_t)0xfec0,
    (int16_t)0xfeab, (int16_t)0x3c85, (int16_t)0x4611, (int16_t)0xfece,
    (int16_t)0xfeac, (int16_t)0x3a95, (int16_t)0x47ed, (int16_t)0xfedf,
    (int16_t)0xfeb0, (int16_t)0x38a5, (int16_t)0x49c2, (int16_t)0xfef5,
    (int16_t)0xfeb6, (int16_t)0x36b6, (int16_t)0x4b91, (int16_t)0xff0f,
    (int16_t)0xfebd, (int16_t)0x34c8, (int16_t)0x4d57, (int16_t)0xff2e,
    (int16_t)0xfec6, (int16_t)0x32dc, (int16_t)0x4f14, (int16_t)0xff53,
    (int16_t)0xfed0, (int16_t)0x30f3, (int16_t)0x50c7, (int16_t)0xff7c,
    (int16_t)0xfedb, (int16_t)0x2f0d, (int16_t)0x5270, (int16_t)0xffac,
    (int16_t)0xfee8, (int16_t)0x2d2c, (int16_t)0x540d, (int16_t)0xffe2,
    (int16_t)0xfef4, (int16_t)0x2b50, (int16_t)0x559d, (int16_t)0x001f,
    (int16_t)0xff02, (int16_t)0x297a, (int16_t)0x5720, (int16_t)0x0063,
    (int16_t)0xff10, (int16_t)0x27a9, (int16_t)0x5896, (int16_t)0x00ae,
    (int16_t)0xff1e, (int16_t)0x25e0, (int16_t)0x59fc, (int16_t)0x0101,
    (int16_t)0xff2c, (int16_t)0x241e, (int16_t)0x5b53, (int16_t)0x015b,
    (int16_t)0xff3a, (int16_t)0x2264, (int16_t)0x5c9a, (int16_t)0x01be,
    (int16_t)0xff48, (int16_t)0x20b3, (int16_t)0x5dd0, (int16_t)0x022a,
    (int16_t)0xff56, (int16_t)0x1f0b, (int16_t)0x5ef5, (int16_t)0x029f,
    (int16_t)0xff64, (int16_t)0x1d6c, (int16_t)0x6007, (int16_t)0x031c,
    (int16_t)0xff71, (int16_t)0x1bd7, (int16_t)0x6106, (int16_t)0x03a4,
    (int16_t)0xff7e, (int16_t)0x1a4c, (int16_t)0x61f3, (int16_t)0x0435,
    (int16_t)0xff8a, (int16_t)0x18cb, (int16_t)0x62cb, (int16_t)0x04d1,
    (int16_t)0xff96, (int16_t)0x1756, (int16_t)0x638f, (int16_t)0x0577,
    (int16_t)0xffa1, (int16_t)0x15eb, (int16_t)0x643f, (int16_t)0x0628,
    (int16_t)0xffac, (int16_t)0x148c, (int16_t)0x64d9, (int16_t)0x06e4,
    (int16_t)0xffb6, (int16_t)0x1338, (int16_t)0x655e, (int16_t)0x07ab,
    (int16_t)0xffbf, (int16_t)0x11f0, (int16_t)0x65cd, (int16_t)0x087d,
    (int16_t)0xffc8, (int16_t)0x10b4, (int16_t)0x6626, (int16_t)0x095a,
    (int16_t)0xffd0, (int16_t)0x0f83, (int16_t)0x6669, (int16_t)0x0a44,
    (int16_t)0xffd8, (int16_t)0x0e5f, (int16_t)0x6696, (int16_t)0x0b39,
    (int16_t)0xffdf, (int16_t)0x0d46, (int16_t)0x66ad, (int16_t)0x0c39
};

static uint8_t alist_buffer[0x1000];
static struct alist_audio_t alist_audio;

static inline size_t align(size_t x, size_t amount) {
    --amount;
    return (x + amount) & ~amount;
}

static inline int16_t clamp_s16(int32_t v) {
    return v < -32768 ? -32768 : v > 32767 ? 32767 : v;
}

static inline int16_t sample_mix(int16_t dst, int16_t src, int16_t gain) {
    int32_t src_modified = (src * gain) >> 15;
    return clamp_s16(dst + src_modified);
}

void aClearBuffer(uint64_t *cmd, uint16_t dmem, uint16_t count) {
    dmem += DMEM_BASE;
    //assert(align(count, 16) == count);
    count = align(count, 16);
    memset(alist_buffer + dmem, 0, count);
}

void aSetBuffer(uint64_t *cmd, uint8_t flags, uint16_t dmemin, uint16_t dmemout, uint16_t count) {
    if (flags & A_AUX) {
        // Parameter names are not really correct for A_AUX
        alist_audio.dry_right = dmemin + DMEM_BASE;
        alist_audio.wet_left = dmemout + DMEM_BASE;
        alist_audio.wet_right = count + DMEM_BASE;
    } else {
        alist_audio.in = dmemin + DMEM_BASE;
        alist_audio.out = dmemout + DMEM_BASE;
        alist_audio.count = count;
    }
}

void aLoadBuffer(uint64_t *cmd, uint16_t *addr) {
    // addr &= ~7
    memcpy(alist_buffer + alist_audio.in, addr, alist_audio.count);
}

void aSaveBuffer(uint64_t *cmd, uint16_t *addr) {
    memcpy(addr, alist_buffer + alist_audio.out, alist_audio.count);
}

void aDMEMMove(uint64_t *cmd, uint16_t dmemin, uint16_t dmemout, uint16_t count) {
    dmemin += DMEM_BASE;
    dmemout += DMEM_BASE;
    //assert(align(count, 16) == count);
    count = align(count, 16); // Microcode does this
    memmove(alist_buffer + dmemout, alist_buffer + dmemin, count);
}

void aMix(uint64_t *cmd, uint8_t flags, uint16_t gain, uint16_t dmemin, uint16_t dmemout) {
    dmemin += DMEM_BASE;
    dmemout += DMEM_BASE;
    
    // originally count is rounded up to nearest 32 bytes
    
    int16_t *dst = (int16_t*)(alist_buffer + dmemout);
    const int16_t *src = (const int16_t*)(alist_buffer + dmemin);
    size_t count = alist_audio.count >> 1;
    count = align(count, 16);
    
    while (count != 0) {
        *dst = sample_mix(*dst, *src, gain);
        ++dst;
        ++src;
        --count;
    }
}

static inline int16_t ramp_step(struct ramp_t* ramp) {
    int target_reached;

    ramp->value += ramp->step;

    target_reached = (ramp->step <= 0)
        ? (ramp->value <= ramp->target)
        : (ramp->value >= ramp->target);

    if (target_reached)
    {
        ramp->value = ramp->target;
        ramp->step  = 0;
    }

    return (int16_t)(ramp->value >> 16);
}

void aEnvMixer(uint64_t *cmd, uint8_t flags, uint16_t *addr) {
    size_t n = (flags & A_AUX) ? 4 : 2;
    
    const int16_t *const in = (int16_t*)(alist_buffer + alist_audio.in);
    int16_t *const dl = (int16_t*)(alist_buffer + alist_audio.out);
    int16_t *const dr = (int16_t*)(alist_buffer + alist_audio.dry_right);
    int16_t *const wl = (int16_t*)(alist_buffer + alist_audio.wet_left);
    int16_t *const wr = (int16_t*)(alist_buffer + alist_audio.wet_right);
    
    struct ramp_t ramps[2];
    int32_t exp_seq[2];
    int32_t exp_rates[2];
    int16_t dry;
    int16_t wet;
    
    uint32_t ptr = 0;
    uint32_t x, y;
    uint32_t count = alist_audio.count;
    struct env_mix_save_buffer_t *s = (struct env_mix_save_buffer_t*)addr;
    
    if (flags & A_INIT) {
        ramps[0].value  = (alist_audio.vol[0] << 16);
        ramps[1].value  = (alist_audio.vol[1] << 16);
        ramps[0].target = (alist_audio.target[0] << 16);
        ramps[1].target = (alist_audio.target[1] << 16);
        exp_rates[0]    = alist_audio.rate[0];
        exp_rates[1]    = alist_audio.rate[1];
        exp_seq[0]      = (alist_audio.vol[0] * alist_audio.rate[0]);
        exp_seq[1]      = (alist_audio.vol[1] * alist_audio.rate[1]);
        dry = alist_audio.dry;
        wet = alist_audio.wet;
    } else {
        wet = s->wet;
        dry = s->dry;
        ramps[0].target = s->ramp_targets[0];
        ramps[1].target = s->ramp_targets[1];
        exp_rates[0] = s->exp_rates[0];
        exp_rates[1] = s->exp_rates[1];
        exp_seq[0] = s->exp_seq[0];
        exp_seq[1] = s->exp_seq[1];
        ramps[0].value = s->ramp_values[0];
        ramps[1].value = s->ramp_values[1];
    }
    
    /* init which ensure ramp.step != 0 iff ramp.value == ramp.target */
    ramps[0].step = ramps[0].target - ramps[0].value;
    ramps[1].step = ramps[1].target - ramps[1].value;
    
    for (y = 0; y < count; y += 16)
    {
        if (ramps[0].step != 0)
        {
            exp_seq[0] = ((int64_t)exp_seq[0]*(int64_t)exp_rates[0]) >> 16;
            ramps[0].step = (exp_seq[0] - ramps[0].value) >> 3;
        }

        if (ramps[1].step != 0)
        {
            exp_seq[1] = ((int64_t)exp_seq[1]*(int64_t)exp_rates[1]) >> 16;
            ramps[1].step = (exp_seq[1] - ramps[1].value) >> 3;
        }

        for (x = 0; x < 8; ++x) {
            int16_t l_vol = ramp_step(&ramps[0]);
            int16_t r_vol = ramp_step(&ramps[1]);
            int16_t in_sample = in[ptr];
            
            dl[ptr] = sample_mix(dl[ptr], in_sample, clamp_s16((l_vol * dry + 0x4000) >> 15));
            dr[ptr] = sample_mix(dr[ptr], in_sample, clamp_s16((r_vol * dry + 0x4000) >> 15));
            if (n == 4) {
                wl[ptr] = sample_mix(wl[ptr], in_sample, clamp_s16((l_vol * wet + 0x4000) >> 15));
                wr[ptr] = sample_mix(wr[ptr], in_sample, clamp_s16((r_vol * wet + 0x4000) >> 15));
            }
            ++ptr;
        }
    }
    
    s->wet = wet;
    s->dry = dry;
    s->ramp_targets[0] = ramps[0].target;
    s->ramp_targets[1] = ramps[1].target;
    s->exp_rates[0] = exp_rates[0];
    s->exp_rates[1] = exp_rates[1];
    s->exp_seq[0] = exp_seq[0];
    s->exp_seq[1] = exp_seq[1];
    s->ramp_values[0] = ramps[0].value;
    s->ramp_values[1] = ramps[1].value;
}

void aResample(uint64_t *cmd, uint8_t flags, uint16_t pitch, uint16_t *state_addr) {
    int16_t *dst = (int16_t*)(alist_buffer + alist_audio.out);
    int16_t *src = (int16_t*)(alist_buffer + alist_audio.in);
    size_t count = alist_audio.count >> 1;
    uint32_t pitch_accumulator = 0;
    
    count = align(count, 8);
    
    src -= 4;
    
    if (flags & A_INIT) {
        memset(src, 0, 4 * sizeof(int16_t));
    } else {
        memcpy(src, state_addr, 4 * sizeof(int16_t));
        pitch_accumulator = state_addr[4];
    }
    
    while (count != 0) {
        const int16_t *lut = RESAMPLE_LUT + ((pitch_accumulator & 0xfc00) >> 8);
        
        *dst++ = clamp_s16((src[0] * lut[0] + src[1] * lut[1] + src[2] * lut[2] + src[3] * lut[3]) >> 15);
        pitch_accumulator += (pitch << 1);
        src += pitch_accumulator >> 16;
        pitch_accumulator &= 0xffff;
        --count;
    }
    
    memcpy(state_addr, src, 4 * sizeof(int16_t));
    state_addr[4] = pitch_accumulator;
}

void aInterleave(uint64_t *cmd, uint16_t inL, uint16_t inR) {
    inL += DMEM_BASE;
    inR += DMEM_BASE;
    
    int16_t *dst = (int16_t*)(alist_buffer + alist_audio.out);
    int16_t *srcL = (int16_t*)(alist_buffer + inL);
    int16_t *srcR = (int16_t*)(alist_buffer + inR);
    
    size_t count = alist_audio.count >> 2;
    
    count = align(count, 4);
    
    // Unroll a bit
    while (count != 0) {
        int16_t l1 = *srcL++;
        int16_t l2 = *srcL++;
        int16_t r1 = *srcR++;
        int16_t r2 = *srcR++;
        
        *dst++ = l1;
        *dst++ = r1;
        *dst++ = l2;
        *dst++ = r2;
        
        --count;
    }
}

// These two share the same opcode but parameters and what they do are different depending on flags
void aSetVolume(uint64_t *cmd, uint8_t flags, uint16_t vol, uint16_t voltgt, uint16_t volrate) {
    if (flags & A_AUX) {
        // Parameter names are not really correct for A_AUX
        alist_audio.dry = vol;
        alist_audio.wet = volrate;
    } else {
        size_t lr = (flags & A_LEFT) ? 0 : 1;
        
        assert(flags & A_VOL);
        alist_audio.vol[lr] = vol;
    }
}
void aSetVolume32(uint64_t *cmd, uint8_t flags, uint16_t voltgt, uint32_t volrate) {
    size_t lr = (flags & A_LEFT) ? 0 : 1;
    
    assert(!(flags & A_VOL) && !(flags & A_AUX));
    alist_audio.target[lr] = voltgt;
    alist_audio.rate[lr] = volrate;
}

void aSetLoop(uint64_t *cmd, uint16_t *addr) {
    alist_audio.loop = addr;
}

void aLoadADPCM(uint64_t *cmd, uint16_t count, uint16_t *addr) {
    assert(align(count, 8) == count);
    memcpy(alist_audio.table, addr, count);
}

static inline int16_t adpcm_predict_sample(uint8_t byte, uint8_t mask,
        unsigned lshift, unsigned rshift) {
    int16_t sample = (uint16_t)(byte & mask) << lshift;
    sample >>= rshift; /* signed */
    return sample;
}

static unsigned int adpcm_predict_frame_4bits(int16_t* dst, uint8_t* src, uint8_t scale) {
    unsigned int i;
    unsigned int rshift = (scale < 12) ? 12 - scale : 0;

    for(i = 0; i < 8; ++i) {
        uint8_t byte = *src++;

        *(dst++) = adpcm_predict_sample(byte, 0xf0,  8, rshift);
        *(dst++) = adpcm_predict_sample(byte, 0x0f, 12, rshift);
    }

    return 8;
}

static int32_t rdot(size_t n, const int16_t *x, const int16_t *y) {
    int32_t accu = 0;

    y += n;

    while (n != 0) {
        accu += *(x++) * *(--y);
        --n;
    }

    return accu;
}

static void adpcm_compute_residuals(int16_t* dst, const int16_t* src,
        const int16_t* cb_entry, const int16_t* last_samples, size_t count) {
    const int16_t* const book1 = cb_entry;
    const int16_t* const book2 = cb_entry + 8;

    const int16_t l1 = last_samples[0];
    const int16_t l2 = last_samples[1];

    size_t i;

    assert(count <= 8);

    for(i = 0; i < count; ++i) {
        int32_t accu = (int32_t)src[i] << 11;
        accu += book1[i]*l1 + book2[i]*l2 + rdot(i, book2, src);
        dst[i] = clamp_s16(accu >> 11);
   }
}

void aADPCMdec(uint64_t *cmd, uint8_t flags, uint16_t *last_frame_addr) {
    int16_t *dst = (int16_t*)(alist_buffer + alist_audio.out);
    uint8_t *src = alist_buffer + alist_audio.in;
    size_t count = alist_audio.count;
    int16_t last_frame[16];

    count = align(count, 32);
    assert((count & 0x1f) == 0);
    
    if (flags & A_INIT) {
        memset(last_frame, 0, sizeof(last_frame));
    } else {
        memcpy(last_frame, ((flags & A_LOOP) ? alist_audio.loop : last_frame_addr), sizeof(last_frame));
    }
    
    memcpy(dst, last_frame, sizeof(last_frame));
    dst += 16;
    
    while (count != 0) {
        int16_t frame[16];
        uint8_t code = *src++;
        uint8_t scale = code >> 4;
        const int16_t *const cb_entry = alist_audio.table + ((code & 0xf) * 16);
        
        src += adpcm_predict_frame_4bits(frame, src, scale);
        
        adpcm_compute_residuals(last_frame, frame, cb_entry, last_frame + 14, 8);
        adpcm_compute_residuals(last_frame + 8, frame + 8, cb_entry, last_frame + 6, 8);
        
        memcpy(dst, last_frame, sizeof(last_frame));
        dst += 16;
        
        count -= 32;
    }
    
    memcpy(last_frame_addr, last_frame, sizeof(last_frame));
}
