#ifndef CHEATS_MENU_H
#define CHEATS_MENU_H

#include <PR/ultratypes.h>

#include "types.h"
#include "../../include/libc/stdlib.h"

static const u8 optsSeqStr[][64] = {
    { TEXT_OPT_SEQ1 },
    { TEXT_OPT_SEQ2 },
    { TEXT_OPT_SEQ3 },
    { TEXT_OPT_SEQ4 },
    { TEXT_OPT_SEQ5 },
    { TEXT_OPT_SEQ6 },
    { TEXT_OPT_SEQ7 },
    { TEXT_OPT_SEQ8 },
    { TEXT_OPT_SEQ9 },
    { TEXT_OPT_SEQ10 },
    { TEXT_OPT_SEQ11 },
    { TEXT_OPT_SEQ12 },
    { TEXT_OPT_SEQ13 },
    { TEXT_OPT_SEQ14 },
    { TEXT_OPT_SEQ15 },
    { TEXT_OPT_SEQ16 },
    { TEXT_OPT_SEQ17 },
    { TEXT_OPT_SEQ18 },
    { TEXT_OPT_SEQ19 },
};

static const u8 *SeqChoices[] = {
    optsSeqStr[0],
    optsSeqStr[1],
    optsSeqStr[2],
    optsSeqStr[3],
    optsSeqStr[4],
    optsSeqStr[5],
    optsSeqStr[6],
    optsSeqStr[7],
    optsSeqStr[8],
    optsSeqStr[9],
    optsSeqStr[10],
    optsSeqStr[11],
    optsSeqStr[12],
    optsSeqStr[13],
    optsSeqStr[14],
    optsSeqStr[15],
    optsSeqStr[16],
    optsSeqStr[17],
    optsSeqStr[18],
}
;

static const u8 optsCheatsStr[][64] = {
    { TEXT_OPT_CHEAT1 },
    { TEXT_OPT_CHEAT2 },
    { TEXT_OPT_CHEAT3 },
    { TEXT_OPT_CHEAT4 },
    { TEXT_OPT_CHEAT5 },
    { TEXT_OPT_CHEAT6 },
    { TEXT_OPT_CHEAT7 },
    { TEXT_OPT_CHEAT8 },
    { TEXT_OPT_CHEAT9 },
    { TEXT_OPT_SPDDPS },
    { TEXT_OPT_TPF },
    { TEXT_OPT_JB },
    { TEXT_OPT_JBC },
    { TEXT_OPT_QUIKEND },
    { TEXT_OPT_HURT },
    { TEXT_OPT_CANN },
    { TEXT_OPT_AWK },
    { TEXT_OPT_SHELL },
    { TEXT_OPT_BOB },
    { TEXT_OPT_SPAMBA },
    { TEXT_OPT_SWIM },
    { TEXT_OPT_WING_CAP },
    { TEXT_OPT_METAL_CAP },
    { TEXT_OPT_VANISH_CAP },
    { TEXT_OPT_REMOVE_CAP },
    { TEXT_OPT_DCM },
    { TEXT_OPT_NORMAL_CAP },
    { TEXT_OPT_BLJ },
};

static const u8 optsHurtCheatStr[][32] = {
    { TEXT_OPT_HURTCHT1 },
    { TEXT_OPT_HURTCHT2 },
    { TEXT_OPT_HURTCHT3 },
    { TEXT_OPT_HURTCHT4 },
};

static const u8 optsSpamCheatStr[][32] = {
    { TEXT_OPT_SPAMCHT1 },
    { TEXT_OPT_SPAMCHT2 },
    { TEXT_OPT_SPAMCHT3 },
    { TEXT_OPT_SPAMCHT4 },
    { TEXT_OPT_SPAMCHT5 },
    { TEXT_OPT_SPAMCHT6 },
    { TEXT_OPT_SPAMCHT7 },
    { TEXT_OPT_SPAMCHT8 },
    { TEXT_OPT_SPAMCHT9 },
    { TEXT_OPT_SPAMCHT10 },
    { TEXT_OPT_SPAMCHT11 },
    { TEXT_OPT_SPAMCHT12 },
    { TEXT_OPT_SPAMCHT13 },
    { TEXT_OPT_SPAMCHT14 },
};

static const u8 optsBLJCheatStr[][32] = {
    { TEXT_OPT_BLJCHT1 },
    { TEXT_OPT_BLJCHT2 },
    { TEXT_OPT_BLJCHT3 },
    { TEXT_OPT_BLJCHT4 },
    { TEXT_OPT_BLJCHT5 },
    { TEXT_OPT_BLJCHT6 },
    { TEXT_OPT_BLJCHT7 },
    { TEXT_OPT_BLJCHT8 },
    { TEXT_OPT_BLJCHT9 },
    { TEXT_OPT_BLJCHT10 },
    { TEXT_OPT_BLJCHT11 },
    { TEXT_OPT_BLJCHT12 },
    { TEXT_OPT_BLJCHT13 },
};

static const u8 *HurtCheatChoices[] = {
    optsHurtCheatStr[0],
    optsHurtCheatStr[1],
    optsHurtCheatStr[2],
    optsHurtCheatStr[3],
};

static const u8 * SpamCheatChoices[] = {
    optsSpamCheatStr[0],
    optsSpamCheatStr[1],
    optsSpamCheatStr[2],
    optsSpamCheatStr[3],
    optsSpamCheatStr[4],
    optsSpamCheatStr[5],
    optsSpamCheatStr[6],
    optsSpamCheatStr[7],
    optsSpamCheatStr[8],
    optsSpamCheatStr[9],
    optsSpamCheatStr[10],
    optsSpamCheatStr[11],
    optsSpamCheatStr[12],
    optsSpamCheatStr[13],
};

static const u8* bljCheatChoices[] = {
    optsBLJCheatStr[0],
    optsBLJCheatStr[1],
    optsBLJCheatStr[2],
    optsBLJCheatStr[3],
    optsBLJCheatStr[4],
    optsBLJCheatStr[5],
    optsBLJCheatStr[6],
    optsBLJCheatStr[7],
    optsBLJCheatStr[8],
    optsBLJCheatStr[9],
    optsBLJCheatStr[10],
    optsBLJCheatStr[11],
    optsBLJCheatStr[12],
};

#endif // CHEATS_MENU_H
