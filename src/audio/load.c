#include <ultra64.h>

#include "data.h"
#include "external.h"
#include "heap.h"
#include "load.h"
#include "seqplayer.h"

#include "pc/platform.h"
#include "pc/fs/fs.h"

#define ALIGN16(val) (((val) + 0xF) & ~0xF)

struct SharedDma {
    /*0x0*/ u8 *buffer;       // target, points to pre-allocated buffer
    /*0x4*/ uintptr_t source; // device address
    /*0x8*/ u16 sizeUnused;   // set to bufSize, never read
    /*0xA*/ u16 bufSize;      // size of buffer
    /*0xC*/ u8 unused2;       // set to 0, never read
    /*0xD*/ u8 reuseIndex;    // position in sSampleDmaReuseQueue1/2, if ttl == 0
    /*0xE*/ u8 ttl;           // duration after which the DMA can be discarded
};                            // size = 0x10

// EU only
void port_eu_init(void);

struct Note *gNotes;

struct SequencePlayer gSequencePlayers[SEQUENCE_PLAYERS];
struct SequenceChannel gSequenceChannels[SEQUENCE_CHANNELS];
struct SequenceChannelLayer gSequenceLayers[SEQUENCE_LAYERS];

struct SequenceChannel gSequenceChannelNone;
struct AudioListItem gLayerFreeList;
struct NotePool gNoteFreeLists;

OSMesgQueue gCurrAudioFrameDmaQueue;
OSMesg gCurrAudioFrameDmaMesgBufs[AUDIO_FRAME_DMA_QUEUE_SIZE];
OSIoMesg gCurrAudioFrameDmaIoMesgBufs[AUDIO_FRAME_DMA_QUEUE_SIZE];

OSMesgQueue gAudioDmaMesgQueue;
OSMesg gAudioDmaMesg;
OSIoMesg gAudioDmaIoMesg;

struct SharedDma sSampleDmas[0x60];
u32 gSampleDmaNumListItems; // sh: 0x803503D4
u32 sSampleDmaListSize1; // sh: 0x803503D8
u32 sUnused80226B40; // set to 0, never read, sh: 0x803503DC

// Circular buffer of DMAs with ttl = 0. tail <= head, wrapping around mod 256.
u8 sSampleDmaReuseQueue1[256];
u8 sSampleDmaReuseQueue2[256];
u8 sSampleDmaReuseQueueTail1;
u8 sSampleDmaReuseQueueTail2;
u8 sSampleDmaReuseQueueHead1; // sh: 0x803505E2
u8 sSampleDmaReuseQueueHead2; // sh: 0x803505E3

// bss correct up to here

ALSeqFile *gSeqFileHeader;
ALSeqFile *gAlCtlHeader;
ALSeqFile *gAlTbl;
u8 *gAlBankSets;
u16 gSequenceCount;

struct CtlEntry *gCtlEntries; // sh: 0x803505F8

s32 gAiFrequency;

u32 sDmaBufSize;
s32 gMaxAudioCmds;
s32 gMaxSimultaneousNotes;
s32 gSamplesPerFrameTarget;
s32 gMinAiBufferLength;
s16 gTempoInternalToExternal;
s8 gAudioUpdatesPerFrame;
s8 gSoundMode;

extern u64 gAudioGlobalsStartMarker;
extern u64 gAudioGlobalsEndMarker;

extern u8 gSoundDataADSR[]; // sound_data.ctl
extern u8 gSoundDataRaw[];  // sound_data.tbl
extern u8 gMusicData[];     // sequences.s
extern u8 gBankSetsData[];  // bank_sets.s

ALSeqFile *get_audio_file_header(s32 arg0);

/**
 * Performs an immediate DMA copy
 */
void audio_dma_copy_immediate(uintptr_t devAddr, void *vAddr, size_t nbytes) {
    osInvalDCache(vAddr, nbytes);
    osPiStartDma(&gAudioDmaIoMesg, OS_MESG_PRI_HIGH, OS_READ, devAddr, vAddr, nbytes,
                 &gAudioDmaMesgQueue);
    osRecvMesg(&gAudioDmaMesgQueue, NULL, OS_MESG_BLOCK);
}

/**
 * Performs an asynchronus (normal priority) DMA copy
 */
void audio_dma_copy_async(uintptr_t devAddr, void *vAddr, size_t nbytes, OSMesgQueue *queue, OSIoMesg *mesg) {
    osInvalDCache(vAddr, nbytes);
    osPiStartDma(mesg, OS_MESG_PRI_NORMAL, OS_READ, devAddr, vAddr, nbytes, queue);
}

/**
 * Performs a partial asynchronous (normal priority) DMA copy. This is limited
 * to 0x1000 bytes transfer at once.
 */
void audio_dma_partial_copy_async(uintptr_t *devAddr, u8 **vAddr, ssize_t *remaining, OSMesgQueue *queue, OSIoMesg *mesg) {
    ssize_t transfer = (*remaining < 0x1000 ? *remaining : 0x1000);
    *remaining -= transfer;
    osInvalDCache(*vAddr, transfer);
    osPiStartDma(mesg, OS_MESG_PRI_NORMAL, OS_READ, *devAddr, *vAddr, transfer, queue);
    *devAddr += transfer;
    *vAddr += transfer;
}

void decrease_sample_dma_ttls() {
    u32 i;

    for (i = 0; i < sSampleDmaListSize1; i++) {
        struct SharedDma *temp = sSampleDmas + i;

        if (temp->ttl != 0) {
            temp->ttl--;
            if (temp->ttl == 0) {
                temp->reuseIndex = sSampleDmaReuseQueueHead1;
                sSampleDmaReuseQueue1[sSampleDmaReuseQueueHead1++] = (u8) i;
            }
        }
    }

    for (i = sSampleDmaListSize1; i < gSampleDmaNumListItems; i++) {
        struct SharedDma *temp = sSampleDmas + i;
        if (temp->ttl != 0) {
            temp->ttl--;
            if (temp->ttl == 0) {
                temp->reuseIndex = sSampleDmaReuseQueueHead2;
                sSampleDmaReuseQueue2[sSampleDmaReuseQueueHead2++] = (u8) i;
            }
        }
    }

    sUnused80226B40 = 0;
}

void *dma_sample_data(uintptr_t devAddr, u32 size, s32 arg2, u8 *dmaIndexRef) {
    s32 hasDma = FALSE;
    struct SharedDma *dma;
    uintptr_t dmaDevAddr;
    u32 transfer;
    u32 i;
    u32 dmaIndex;
    ssize_t bufferPos;
    UNUSED u32 pad;

    if (arg2 != 0 || *dmaIndexRef >= sSampleDmaListSize1) {
        for (i = sSampleDmaListSize1; i < gSampleDmaNumListItems; i++) {
            dma = sSampleDmas + i;
            bufferPos = devAddr - dma->source;
            if (0 <= bufferPos && (size_t) bufferPos <= dma->bufSize - size) {
                // We already have a DMA request for this memory range.
                if (dma->ttl == 0 && sSampleDmaReuseQueueTail2 != sSampleDmaReuseQueueHead2) {
                    // Move the DMA out of the reuse queue, by swapping it with the
                    // tail, and then incrementing the tail.
                    if (dma->reuseIndex != sSampleDmaReuseQueueTail2) {
                        sSampleDmaReuseQueue2[dma->reuseIndex] =
                            sSampleDmaReuseQueue2[sSampleDmaReuseQueueTail2];
                        sSampleDmas[sSampleDmaReuseQueue2[sSampleDmaReuseQueueTail2]].reuseIndex =
                            dma->reuseIndex;
                    }
                    sSampleDmaReuseQueueTail2++;
                }
                dma->ttl = 60;
                *dmaIndexRef = (u8) i;
                return (devAddr - dma->source) + dma->buffer;
            }
        }

        if (sSampleDmaReuseQueueTail2 != sSampleDmaReuseQueueHead2 && arg2 != 0) {
            // Allocate a DMA from reuse queue 2. This queue can be empty, since
            // TTL 60 is pretty large.
            dmaIndex = sSampleDmaReuseQueue2[sSampleDmaReuseQueueTail2];
            sSampleDmaReuseQueueTail2++;
            dma = sSampleDmas + dmaIndex;
            hasDma = TRUE;
        }
    } else {
        dma = sSampleDmas + *dmaIndexRef;
        bufferPos = devAddr - dma->source;
        if (0 <= bufferPos && (size_t) bufferPos <= dma->bufSize - size) {
            // We already have DMA for this memory range.
            if (dma->ttl == 0) {
                // Move the DMA out of the reuse queue, by swapping it with the
                // tail, and then incrementing the tail.
                if (dma->reuseIndex != sSampleDmaReuseQueueTail1) {
                    sSampleDmaReuseQueue1[dma->reuseIndex] =
                        sSampleDmaReuseQueue1[sSampleDmaReuseQueueTail1];
                    sSampleDmas[sSampleDmaReuseQueue1[sSampleDmaReuseQueueTail1]].reuseIndex =
                        dma->reuseIndex;
                }
                sSampleDmaReuseQueueTail1++;
            }
            dma->ttl = 2;
            return (devAddr - dma->source) + dma->buffer;
        }
    }

    if (!hasDma) {
        // Allocate a DMA from reuse queue 1. This queue will hopefully never
        // be empty, since TTL 2 is so small.
        dmaIndex = sSampleDmaReuseQueue1[sSampleDmaReuseQueueTail1++];
        dma = sSampleDmas + dmaIndex;
        hasDma = TRUE;
    }

    transfer = dma->bufSize;
    dmaDevAddr = devAddr & ~0xF;
    dma->ttl = 2;
    dma->source = dmaDevAddr;
    dma->sizeUnused = transfer;
    osInvalDCache(dma->buffer, transfer);
    gCurrAudioFrameDmaCount++;
    osPiStartDma(&gCurrAudioFrameDmaIoMesgBufs[gCurrAudioFrameDmaCount - 1], OS_MESG_PRI_NORMAL,
                 OS_READ, dmaDevAddr, dma->buffer, transfer, &gCurrAudioFrameDmaQueue);
    *dmaIndexRef = dmaIndex;
    return dma->buffer + (devAddr - dmaDevAddr);
}


void init_sample_dma_buffers(UNUSED s32 arg0) {
    s32 i, j;
    sDmaBufSize = 144 * 9;

    for (i = 0; i < gMaxSimultaneousNotes * 3; i++) {
        sSampleDmas[gSampleDmaNumListItems].buffer = soundAlloc(&gNotesAndBuffersPool, sDmaBufSize);
        if (sSampleDmas[gSampleDmaNumListItems].buffer == NULL) {
            goto out1;
        }
        sSampleDmas[gSampleDmaNumListItems].source = 0;
        sSampleDmas[gSampleDmaNumListItems].sizeUnused = 0;
        sSampleDmas[gSampleDmaNumListItems].unused2 = 0;
        sSampleDmas[gSampleDmaNumListItems].ttl = 0;
        sSampleDmas[gSampleDmaNumListItems].bufSize = sDmaBufSize;
        gSampleDmaNumListItems++;
    }
out1:

    for (i = 0; (u32) i < gSampleDmaNumListItems; i++) {
        sSampleDmaReuseQueue1[i] = (u8) i;
        sSampleDmas[i].reuseIndex = (u8) i;
    }

    for (j = gSampleDmaNumListItems; j < 0x100; j++) {
        sSampleDmaReuseQueue1[j] = 0;
    }

    sSampleDmaReuseQueueTail1 = 0;
    sSampleDmaReuseQueueHead1 = (u8) gSampleDmaNumListItems;
    sSampleDmaListSize1 = gSampleDmaNumListItems;

    sDmaBufSize = 160 * 9;
    for (i = 0; i < gMaxSimultaneousNotes; i++) {
        sSampleDmas[gSampleDmaNumListItems].buffer = soundAlloc(&gNotesAndBuffersPool, sDmaBufSize);
        if (sSampleDmas[gSampleDmaNumListItems].buffer == NULL) {
            goto out2;
        }
        sSampleDmas[gSampleDmaNumListItems].source = 0;
        sSampleDmas[gSampleDmaNumListItems].sizeUnused = 0;
        sSampleDmas[gSampleDmaNumListItems].unused2 = 0;
        sSampleDmas[gSampleDmaNumListItems].ttl = 0;
        sSampleDmas[gSampleDmaNumListItems].bufSize = sDmaBufSize;
        gSampleDmaNumListItems++;
    }
out2:
    for (i = sSampleDmaListSize1; (u32) i < gSampleDmaNumListItems; i++) {
        sSampleDmaReuseQueue2[i - sSampleDmaListSize1] = (u8) i;
        sSampleDmas[i].reuseIndex = (u8)(i - sSampleDmaListSize1);
    }

    // This probably meant to touch the range size1..size2 as well... but it
    // doesn't matter, since these values are never read anyway.
    for (j = gSampleDmaNumListItems; j < 0x100; j++) {
        sSampleDmaReuseQueue2[j] = sSampleDmaListSize1;
    }

    sSampleDmaReuseQueueTail2 = 0;
    sSampleDmaReuseQueueHead2 = gSampleDmaNumListItems - sSampleDmaListSize1;
}

#ifndef static
// Keep supporting the good old "#define static" hack.
#undef static
#endif
// This function gets optimized out on US due to being static and never called
static void patch_sound(UNUSED struct AudioBankSound *sound, UNUSED u8 *memBase, UNUSED u8 *offsetBase) {
    struct AudioBankSample *sample;
    void *patched;
    UNUSED u8 *mem; // unused on US

#define PATCH(x, base) (patched = (void *)((uintptr_t) (x) + (uintptr_t) base))

    if (sound->sample != NULL) {
        sample = sound->sample = PATCH(sound->sample, memBase);
        if (sample->loaded == 0) {
            sample->sampleAddr = PATCH(sample->sampleAddr, offsetBase);
            sample->loop = PATCH(sample->loop, memBase);
            sample->book = PATCH(sample->book, memBase);
            sample->loaded = 1;
        }
    }

#undef PATCH
}

#define PATCH_SOUND(_sound, mem, offset)                                                  \
{                                                                                         \
    struct AudioBankSound *sound = _sound;                                                \
    struct AudioBankSample *sample;                                                       \
    void *patched;                                                                        \
    if ((*sound).sample != (void *) 0)                                                    \
    {                                                                                     \
        patched = (void *)(((uintptr_t)(*sound).sample) + ((uintptr_t)((u8 *) mem)));     \
        (*sound).sample = patched;                                                        \
        sample = (*sound).sample;                                                         \
        if ((*sample).loaded == 0)                                                        \
        {                                                                                 \
            patched = (void *)(((uintptr_t)(*sample).sampleAddr) + ((uintptr_t) offset)); \
            (*sample).sampleAddr = patched;                                               \
            patched = (void *)(((uintptr_t)(*sample).loop) + ((uintptr_t)((u8 *) mem)));  \
            (*sample).loop = patched;                                                     \
            patched = (void *)(((uintptr_t)(*sample).book) + ((uintptr_t)((u8 *) mem)));  \
            (*sample).book = patched;                                                     \
            (*sample).loaded = 1;                                                         \
        }                                                                                 \
    }                                                                                     \
}

void patch_audio_bank(struct AudioBank *mem, u8 *offset, u32 numInstruments, u32 numDrums) {
    struct Instrument *instrument;
    struct Instrument **itInstrs;
    struct Instrument **end;
    struct AudioBank *temp; // Maybe Shindou also has this; I'm not sure.
    u32 i;
    void *patched;
    struct Drum *drum;
    struct Drum **drums;

#define BASE_OFFSET(x, base) (void *)((uintptr_t) (x) + (uintptr_t) base)
#define PATCH(x, base) (patched = BASE_OFFSET(x, base))
#define PATCH_MEM(x) x = PATCH(x, mem)

    drums = mem->drums;
    if (drums != NULL && numDrums > 0) {
        mem->drums = (void *)((uintptr_t) drums + (uintptr_t) mem);
        if (numDrums > 0) //! unneeded when -sopt is enabled
        for (i = 0; i < numDrums; i++) {
            patched = mem->drums[i];
            if (patched != NULL) {
                drum = PATCH(patched, mem);
                mem->drums[i] = drum;
                if (drum->loaded == 0) {
                    //! copt replaces drum with 'patched' for these two lines
                    PATCH_SOUND(&(*(struct Drum *)patched).sound, mem, offset);
                    patched = (*(struct Drum *)patched).envelope;
                    drum->envelope = BASE_OFFSET(mem, patched);
                    drum->loaded = 1;
                }

            }
        }
    }

    temp = &*mem;
    if (numInstruments > 0) {
        struct Instrument **tempInst;
        itInstrs = temp->instruments;
        tempInst = temp->instruments;
        end = numInstruments + tempInst;

    l2:
        if (*itInstrs != NULL) {
            *itInstrs = BASE_OFFSET(*itInstrs, mem);
            instrument = *itInstrs;

            if (instrument->loaded == 0) {
                PATCH_SOUND(&instrument->lowNotesSound, (u8 *) mem, offset);
                PATCH_SOUND(&instrument->normalNotesSound, (u8 *) mem, offset);
                PATCH_SOUND(&instrument->highNotesSound, (u8 *) mem, offset);
                patched = instrument->envelope;
                instrument->envelope = BASE_OFFSET(mem, patched);
                instrument->loaded = 1;
            }
        }
        itInstrs++;
        //! goto generated by copt, required to match US/JP
        if (end != itInstrs) {
            goto l2;
        }
    }
#undef PATCH_MEM
#undef PATCH
#undef BASE_OFFSET
#undef PATCH_SOUND
}

struct AudioBank *bank_load_immediate(s32 bankId, s32 arg1) {
    UNUSED u32 pad1[4];
    u32 buf[4];
    u32 numInstruments, numDrums;
    struct AudioBank *ret;
    u8 *ctlData;
    s32 alloc;

    // (This is broken if the length is 1 (mod 16), but that never happens --
    // it's always divisible by 4.)
    alloc = gAlCtlHeader->seqArray[bankId].len + 0xf;
    alloc = ALIGN16(alloc);
    alloc -= 0x10;
    ctlData = gAlCtlHeader->seqArray[bankId].offset;
    ret = alloc_bank_or_seq(&gBankLoadedPool, 1, alloc, arg1, bankId);
    if (ret == NULL) {
        return NULL;
    }

    audio_dma_copy_immediate((uintptr_t) ctlData, buf, 0x10);
    numInstruments = buf[0];
    numDrums = buf[1];
    audio_dma_copy_immediate((uintptr_t)(ctlData + 0x10), ret, alloc);
    patch_audio_bank(ret, gAlTbl->seqArray[bankId].offset, numInstruments, numDrums);
    gCtlEntries[bankId].numInstruments = (u8) numInstruments;
    gCtlEntries[bankId].numDrums = (u8) numDrums;
    gCtlEntries[bankId].instruments = ret->instruments;
    gCtlEntries[bankId].drums = ret->drums;
    gBankLoadStatus[bankId] = SOUND_LOAD_STATUS_COMPLETE;
    return ret;
}

struct AudioBank *bank_load_async(s32 bankId, s32 arg1, struct SequencePlayer *seqPlayer) {
    u32 numInstruments, numDrums;
    UNUSED u32 pad1[2];
    u32 buf[4];
    UNUSED u32 pad2;
    size_t alloc;
    struct AudioBank *ret;
    u8 *ctlData;
    OSMesgQueue *mesgQueue;

    alloc = gAlCtlHeader->seqArray[bankId].len + 0xf;
    alloc = ALIGN16(alloc);
    alloc -= 0x10;
    ctlData = gAlCtlHeader->seqArray[bankId].offset;
    ret = alloc_bank_or_seq(&gBankLoadedPool, 1, alloc, arg1, bankId);
    if (ret == NULL) {
        return NULL;
    }

    audio_dma_copy_immediate((uintptr_t) ctlData, buf, 0x10);
    numInstruments = buf[0];
    numDrums = buf[1];
    seqPlayer->loadingBankId = (u8) bankId;
    seqPlayer->loadingBankNumInstruments = numInstruments;
    seqPlayer->loadingBankNumDrums = numDrums;
    seqPlayer->bankDmaCurrMemAddr = (u8 *) ret;
    seqPlayer->loadingBank = ret;
    seqPlayer->bankDmaCurrDevAddr = (uintptr_t)(ctlData + 0x10);
    seqPlayer->bankDmaRemaining = alloc;
    mesgQueue = &seqPlayer->bankDmaMesgQueue;
    osCreateMesgQueue(mesgQueue, &seqPlayer->bankDmaMesg, 1);
    seqPlayer->bankDmaMesg = NULL;
    seqPlayer->bankDmaInProgress = TRUE;
    audio_dma_partial_copy_async(&seqPlayer->bankDmaCurrDevAddr, &seqPlayer->bankDmaCurrMemAddr,
                                 &seqPlayer->bankDmaRemaining, mesgQueue, &seqPlayer->bankDmaIoMesg);
    gBankLoadStatus[bankId] = SOUND_LOAD_STATUS_IN_PROGRESS;
    return ret;
}

void *sequence_dma_immediate(s32 seqId, s32 arg1) {
    s32 seqLength;
    void *ptr;
    u8 *seqData;

    seqLength = gSeqFileHeader->seqArray[seqId].len + 0xf;
    seqLength = ALIGN16(seqLength);
    seqData = gSeqFileHeader->seqArray[seqId].offset;
    ptr = alloc_bank_or_seq(&gSeqLoadedPool, 1, seqLength, arg1, seqId);
    if (ptr == NULL) {
        return NULL;
    }

    audio_dma_copy_immediate((uintptr_t) seqData, ptr, seqLength);
    gSeqLoadStatus[seqId] = SOUND_LOAD_STATUS_COMPLETE;
    return ptr;
}

void *sequence_dma_async(s32 seqId, s32 arg1, struct SequencePlayer *seqPlayer) {
    s32 seqLength;
    void *ptr;
    u8 *seqData;
    OSMesgQueue *mesgQueue;

    seqLength = gSeqFileHeader->seqArray[seqId].len + 0xf;
    seqLength = ALIGN16(seqLength);
    seqData = gSeqFileHeader->seqArray[seqId].offset;
    ptr = alloc_bank_or_seq(&gSeqLoadedPool, 1, seqLength, arg1, seqId);
    if (ptr == NULL) return NULL;

    if (seqLength <= 0x40) {
        // Immediately load short sequenece
        audio_dma_copy_immediate((uintptr_t) seqData, ptr, seqLength);
        gSeqLoadStatus[seqId] = SOUND_LOAD_STATUS_COMPLETE;
    } else {
        audio_dma_copy_immediate((uintptr_t) seqData, ptr, 0x40);
        mesgQueue = &seqPlayer->seqDmaMesgQueue;
        osCreateMesgQueue(mesgQueue, &seqPlayer->seqDmaMesg, 1);
        seqPlayer->seqDmaMesg = NULL;
        seqPlayer->seqDmaInProgress = TRUE;
        audio_dma_copy_async((uintptr_t)(seqData + 0x40), (u8 *) ptr + 0x40, seqLength - 0x40, mesgQueue,
                             &seqPlayer->seqDmaIoMesg);
        gSeqLoadStatus[seqId] = SOUND_LOAD_STATUS_IN_PROGRESS;
    }
    return ptr;
}

u8 get_missing_bank(u32 seqId, s32 *nonNullCount, s32 *nullCount) {
    void *temp;
    u32 bankId;
    u16 offset;
    u8 i;
    u8 ret;

    *nullCount = 0;
    *nonNullCount = 0;
    offset = ((u16 *) gAlBankSets)[seqId] + 1;
    for (i = gAlBankSets[offset - 1], ret = 0; i != 0; i--) {
        offset++;
        bankId = gAlBankSets[offset - 1];

        if (IS_BANK_LOAD_COMPLETE(bankId) == TRUE) {
            temp = get_bank_or_seq(&gBankLoadedPool, 2, gAlBankSets[offset - 1]);
        } else {
            temp = NULL;
        }

        if (temp == NULL) {
            (*nullCount)++;
            ret = bankId;
        } else {
            (*nonNullCount)++;
        }
    }

    return ret;
}
struct AudioBank *load_banks_immediate(s32 seqId, u8 *arg1) {
    void *ret;
    u32 bankId;
    u16 offset;
    u8 i;

    offset = ((u16 *) gAlBankSets)[seqId] + 1;
    for (i = gAlBankSets[offset - 1]; i != 0; i--) {
        offset++;
        bankId = gAlBankSets[offset - 1];

        if (IS_BANK_LOAD_COMPLETE(bankId) == TRUE) {
            ret = get_bank_or_seq(&gBankLoadedPool, 2, gAlBankSets[offset - 1]);
        } else {
            ret = NULL;
        }

        if (ret == NULL) {
            ret = bank_load_immediate(bankId, 2);
        }
    }
    *arg1 = bankId;
    return ret;
}

void preload_sequence(u32 seqId, u8 preloadMask) {
    void *sequenceData;
    u8 temp;

    if (seqId >= gSequenceCount) {
        return;
    }

    gAudioLoadLock = AUDIO_LOCK_LOADING;
    if (preloadMask & PRELOAD_BANKS) {
        load_banks_immediate(seqId, &temp);
    }

    if (preloadMask & PRELOAD_SEQUENCE) {
        // @bug should be IS_SEQ_LOAD_COMPLETE
        if (IS_BANK_LOAD_COMPLETE(seqId) == TRUE) {
            sequenceData = get_bank_or_seq(&gSeqLoadedPool, 2, seqId);
        } else {
            sequenceData = NULL;
        }
        if (sequenceData == NULL && sequence_dma_immediate(seqId, 2) == NULL) {
            gAudioLoadLock = AUDIO_LOCK_NOT_LOADING;
            return;
        }
    }

    gAudioLoadLock = AUDIO_LOCK_NOT_LOADING;
}

void load_sequence_internal(u32 player, u32 seqId, s32 loadAsync);

void load_sequence(u32 player, u32 seqId, s32 loadAsync) {
    if (!loadAsync) {
        gAudioLoadLock = AUDIO_LOCK_LOADING;
    }
    load_sequence_internal(player, seqId, loadAsync);
    if (!loadAsync) {
        gAudioLoadLock = AUDIO_LOCK_NOT_LOADING;
    }
}

void load_sequence_internal(u32 player, u32 seqId, s32 loadAsync) {
    void *sequenceData;
    struct SequencePlayer *seqPlayer = &gSequencePlayers[player];
    UNUSED u32 padding[2];

    if (seqId >= gSequenceCount) {
        return;
    }

    sequence_player_disable(seqPlayer);
    if (loadAsync) {
        s32 numMissingBanks = 0;
        s32 dummy = 0;
        s32 bankId = get_missing_bank(seqId, &dummy, &numMissingBanks);
        if (numMissingBanks == 1) {
            if (bank_load_async(bankId, 2, seqPlayer) == NULL) {
                return;
            }
            // @bug This should set the last bank (i.e. the first in the JSON)
            // as default, not the missing one. This code path never gets
            // taken, though -- all sequence loading is synchronous.
            seqPlayer->defaultBank[0] = bankId;
        } else if (load_banks_immediate(seqId, &seqPlayer->defaultBank[0]) == NULL) {
            return;
        }
    } else if (load_banks_immediate(seqId, &seqPlayer->defaultBank[0]) == NULL) {
        return;
    }

    seqPlayer->seqId = seqId;
    sequenceData = get_bank_or_seq(&gSeqLoadedPool, 2, seqId);
    if (sequenceData == NULL) {
        if (seqPlayer->seqDmaInProgress)
            return;
        if (loadAsync) {
            sequenceData = sequence_dma_async(seqId, 2, seqPlayer);
        } else {
            sequenceData = sequence_dma_immediate(seqId, 2);
        }

        if (sequenceData == NULL) {
            return;
        }
    }

    init_sequence_player(player);
    seqPlayer->scriptState.depth = 0;
    seqPlayer->delay = 0;
    seqPlayer->enabled = TRUE;
    seqPlayer->seqData = sequenceData;
    seqPlayer->scriptState.pc = sequenceData;
}

# define LOAD_DATA(x) load_sound_res((const char *)x)
# include <stdio.h>
# include <stdlib.h>
static inline void *load_sound_res(const char *path) {
    void *data = fs_load_file(path, NULL);
    if (!data) sys_fatal("could not load sound data from '%s'", path);
    // FIXME: figure out where it is safe to free this shit
    //        can't free it immediately after in audio_init()
    return data;
}

void audio_init() {
    UNUSED s8 pad[32];
    u8 buf[0x10];
    s32 i, j, UNUSED k;
    s32 lim1, lim2, lim3;
    u32 size;
    UNUSED u64 *ptr64;
    void *data;
    UNUSED s32 pad2;
    gAudioLoadLock = AUDIO_LOCK_UNINITIALIZED;

    lim1 = gUnusedCount80333EE8;
    for (i = 0; i < lim1; i++) {
        gUnused80226E58[i] = 0;
        gUnused80226E98[i] = 0;
    }

    lim2 = gAudioHeapSize;
    for (i = 0; i <= lim2 / 8 - 1; i++) {
        ((u64 *) gAudioHeap)[i] = 0;
    }


    for (i = 0; i < NUMAIBUFFERS; i++) {
        gAiBufferLengths[i] = 0xa0;
    }

    gAudioFrameCount = 0;
    gAudioTaskIndex = 0;
    gCurrAiBufferIndex = 0;
    gSoundMode = 0;
    gAudioTask = NULL;
    gAudioTasks[0].task.t.data_size = 0;
    gAudioTasks[1].task.t.data_size = 0;
    osCreateMesgQueue(&gAudioDmaMesgQueue, &gAudioDmaMesg, 1);
    osCreateMesgQueue(&gCurrAudioFrameDmaQueue, gCurrAudioFrameDmaMesgBufs,
                      ARRAY_COUNT(gCurrAudioFrameDmaMesgBufs));
    gCurrAudioFrameDmaCount = 0;
    gSampleDmaNumListItems = 0;

    sound_init_main_pools(gAudioInitPoolSize);

    for (i = 0; i < NUMAIBUFFERS; i++) {
        gAiBuffers[i] = soundAlloc(&gAudioInitPool, AIBUFFER_LEN);

        for (j = 0; j < (s32) (AIBUFFER_LEN / sizeof(s16)); j++) {
            gAiBuffers[i][j] = 0;
        }
    }
    audio_reset_session(&gAudioSessionPresets[0]);

    // Load header for sequence data (assets/music_data.sbk.s)
    gSeqFileHeader = (ALSeqFile *) buf;
    data = LOAD_DATA(gMusicData);
    audio_dma_copy_immediate((uintptr_t) data, gSeqFileHeader, 0x10);
    gSequenceCount = gSeqFileHeader->seqCount;
    size = ALIGN16(gSequenceCount * sizeof(ALSeqData) + 4);
    gSeqFileHeader = soundAlloc(&gAudioInitPool, size);
    audio_dma_copy_immediate((uintptr_t) data, gSeqFileHeader, size);
    alSeqFileNew(gSeqFileHeader, data);

    // Load header for CTL (assets/sound_data.ctl.s, i.e. ADSR)
    gAlCtlHeader = (ALSeqFile *) buf;
    data = LOAD_DATA(gSoundDataADSR);
    audio_dma_copy_immediate((uintptr_t) data, gAlCtlHeader, 0x10);
    size = gAlCtlHeader->seqCount * sizeof(ALSeqData) + 4;
    size = ALIGN16(size);
    gCtlEntries = soundAlloc(&gAudioInitPool, gAlCtlHeader->seqCount * sizeof(struct CtlEntry));
    gAlCtlHeader = soundAlloc(&gAudioInitPool, size);
    audio_dma_copy_immediate((uintptr_t) data, gAlCtlHeader, size);
    alSeqFileNew(gAlCtlHeader, data);

    // Load header for TBL (assets/sound_data.tbl.s, i.e. raw data)
    gAlTbl = (ALSeqFile *) buf;
    audio_dma_copy_immediate((uintptr_t) data, gAlTbl, 0x10);
    size = gAlTbl->seqCount * sizeof(ALSeqData) + 4;
    size = ALIGN16(size);
    gAlTbl = soundAlloc(&gAudioInitPool, size);

    data = LOAD_DATA(gSoundDataRaw);
    if(data == NULL || gAlTbl == NULL) {
        printf("Unhandled exception null");
        return;
    }

    audio_dma_copy_immediate((uintptr_t) data, gAlTbl, size);
    alSeqFileNew(gAlTbl, data);

    // Load bank sets for each sequence (assets/bank_sets.s)
    data = LOAD_DATA(gBankSetsData);
    gAlBankSets = soundAlloc(&gAudioInitPool, 0x100);
    audio_dma_copy_immediate((uintptr_t) data, gAlBankSets, 0x100);

    init_sequence_players();
    gAudioLoadLock = AUDIO_LOCK_NOT_LOADING;
}