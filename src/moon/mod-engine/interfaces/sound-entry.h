#ifndef Moon64AudioEntry
#define Moon64AudioEntry
#include <string>

extern "C" {
#include "types.h"
#include "PR/libaudio.h"
#include "audio/internal.h"
}

struct SoundEntry {
    ALSeqFile* seqHeader;
    u16        seqCount;

    ALSeqFile* ctlHeader;
    CtlEntry*  ctlEntries;

    ALSeqFile* tblHeader;
    u8*        bankSets;
};

#endif