#include <ultra64.h>
#include "sm64.h"
#include "segment_symbols.h"
#include "level_commands.h"
#include "levels/intro/header.h"
#include "make_const_nonconst.h"

const LevelScript level_script_entry[] = {
    INIT_LEVEL(),
    SLEEP(/*frames*/ 2),
    BLACKOUT(/*active*/ FALSE),
    SET_REG(/*value*/ 0),
#ifdef SM64_DEBUG
    EXECUTE(/*seg*/ 0x14, /*script*/ _introSegmentRomStart, /*scriptEnd*/ _introSegmentRomEnd, /*entry*/ level_intro_entry_4),
#else
    EXECUTE(/*seg*/ 0x14, /*script*/ _introSegmentRomStart, /*scriptEnd*/ _introSegmentRomEnd, /*entry*/ level_intro_n64),
#endif
    JUMP(/*target*/ level_script_entry),
};
