#ifndef MIXER_H
#define MIXER_H

#include <PR/ultratypes.h>

#undef aSegment
#undef aClearBuffer
#undef aSetBuffer
#undef aLoadBuffer
#undef aSaveBuffer
#undef aDMEMMove
#undef aMix
#undef aEnvMixer
#undef aResample
#undef aInterleave
#undef aSetVolume
#undef aSetVolume32
#undef aSetLoop
#undef aLoadADPCM
#undef aADPCMdec

#define aSegment(pkt, s, b)
void aClearBuffer(UNUSED u64 *cmd, uint16_t dmem, uint16_t count);
void aSetBuffer(UNUSED u64 *cmd, uint8_t flags, uint16_t dmemin, uint16_t dmemout, uint16_t count);
void aLoadBuffer(UNUSED u64 *cmd, uint16_t *addr);
void aSaveBuffer(UNUSED u64 *cmd, uint16_t *addr);
void aDMEMMove(UNUSED u64 *cmd, uint16_t dmemin, uint16_t dmemout, uint16_t count);
void aMix(UNUSED u64 *cmd, uint8_t flags, uint16_t gain, uint16_t dmemin, uint16_t dmemout);
void aEnvMixer(UNUSED u64 *cmd, uint8_t flags, uint16_t *addr);
void aResample(UNUSED u64 *cmd, uint8_t flags, uint16_t pitch, uint16_t *state_addr);
void aInterleave(UNUSED u64 *cmd, uint16_t inL, uint16_t inR);
void aSetVolume(UNUSED u64 *cmd, uint8_t flags, uint16_t vol, uint16_t voltgt, uint16_t volrate);
void aSetVolume32(UNUSED u64 *cmd, uint8_t flags, uint16_t voltgt, uint32_t volrate);
void aSetLoop(UNUSED u64 *cmd, uint16_t *addr);
void aLoadADPCM(UNUSED u64 *cmd, uint16_t count, uint16_t *addr);
void aADPCMdec(UNUSED u64 *cmd, uint8_t flags, uint16_t *last_frame_addr);


#endif
