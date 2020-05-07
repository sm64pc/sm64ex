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
void aClearBuffer(uint64_t *cmd, uint16_t dmem, uint16_t count);
void aSetBuffer(uint64_t *cmd, uint8_t flags, uint16_t dmemin, uint16_t dmemout, uint16_t count);
void aLoadBuffer(uint64_t *cmd, uint16_t *addr);
void aSaveBuffer(uint64_t *cmd, uint16_t *addr);
void aDMEMMove(uint64_t *cmd, uint16_t dmemin, uint16_t dmemout, uint16_t count);
void aMix(uint64_t *cmd, uint8_t flags, uint16_t gain, uint16_t dmemin, uint16_t dmemout);
void aEnvMixer(uint64_t *cmd, uint8_t flags, uint16_t *addr);
void aResample(uint64_t *cmd, uint8_t flags, uint16_t pitch, uint16_t *state_addr);
void aInterleave(uint64_t *cmd, uint16_t inL, uint16_t inR);
void aSetVolume(uint64_t *cmd, uint8_t flags, uint16_t vol, uint16_t voltgt, uint16_t volrate);
void aSetVolume32(uint64_t *cmd, uint8_t flags, uint16_t voltgt, uint32_t volrate);
void aSetLoop(uint64_t *cmd, uint16_t *addr);
void aLoadADPCM(uint64_t *cmd, uint16_t count, uint16_t *addr);
void aADPCMdec(uint64_t *cmd, uint8_t flags, uint16_t *last_frame_addr);


#endif
