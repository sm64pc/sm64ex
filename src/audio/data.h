#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include <PR/ultratypes.h>

#include "internal.h"
#include "types.h"

#define AUDIO_LOCK_UNINITIALIZED 0
#define AUDIO_LOCK_NOT_LOADING 0x76557364
#define AUDIO_LOCK_LOADING 0x19710515

#define NUMAIBUFFERS 3

// constant .data
extern struct AudioSessionSettings gAudioSessionPresets[18];
extern u16 D_80332388[128]; // unused

extern f32 gPitchBendFrequencyScale[255];
extern f32 gNoteFrequencies[128];

extern u8 gDefaultShortNoteVelocityTable[16];
extern u8 gDefaultShortNoteDurationTable[16];
extern s8 gVibratoCurve[16];
extern struct AdsrEnvelope gDefaultEnvelope[3];

extern s16 *gWaveSamples[4];

extern u16 gHeadsetPanQuantization[10];
extern f32 gHeadsetPanVolume[128];
extern f32 gStereoPanVolume[128];
extern f32 gDefaultPanVolume[128];

extern f32 gVolRampingLhs136[128];
extern f32 gVolRampingRhs136[128];
extern f32 gVolRampingLhs144[128];
extern f32 gVolRampingRhs144[128];
extern f32 gVolRampingLhs128[128];
extern f32 gVolRampingRhs128[128];

// non-constant .data
extern s16 gTatumsPerBeat;
extern s8 gUnusedCount80333EE8;
extern s32 gAudioHeapSize; // AUDIO_HEAP_SIZE
extern s32 gAudioInitPoolSize; // AUDIO_INIT_POOL_SIZE
extern volatile s32 gAudioLoadLock;

// .bss
extern volatile s32 gAudioFrameCount;

// number of DMAs performed during this frame
extern volatile s32 gCurrAudioFrameDmaCount;

extern s32 gAudioTaskIndex;
extern s32 gCurrAiBufferIndex;

extern u64 *gAudioCmdBuffers[2];
extern u64 *gAudioCmd;

extern struct SPTask *gAudioTask;
extern struct SPTask gAudioTasks[2];

extern s16 *gAiBuffers[NUMAIBUFFERS];
extern s16 gAiBufferLengths[NUMAIBUFFERS];
#define AIBUFFER_LEN (0xa0 * 16)

extern u32 gUnused80226E58[0x10];
extern u16 gUnused80226E98[0x10];

extern u32 gAudioRandom;
#define UNUSED_COUNT_80333EE8 16
#define AUDIO_HEAP_SIZE 0x31150
#define AUDIO_INIT_POOL_SIZE 0x2500

#endif // AUDIO_DATA_H