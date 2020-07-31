#include <ultra64.h>
#include "sm64.h"
#include "segment_symbols.h"
#include "level_commands.h"

#ifdef IMMEDIATELOAD
#include "levels/menu/header.h"
#else
#include "levels/intro/header.h"
#endif

#include "make_const_nonconst.h"

const LevelScript level_script_entry[] = {
    INIT_LEVEL(),
    SLEEP(/*frames*/ 2),
    BLACKOUT(/*active*/ FALSE),
    SET_REG(/*value*/ 0),
#ifdef IMMEDIATELOAD
    EXECUTE(/*seg*/ 0x14, /*script*/ _introSegmentRomStart, /*scriptEnd*/ _introSegmentRomEnd, /*entry*/ level_main_menu_entry_1),
    JUMP(/*target*/ level_main_menu_entry_1),
#else
    EXECUTE(/*seg*/ 0x14, /*script*/ _introSegmentRomStart, /*scriptEnd*/ _introSegmentRomEnd, /*entry*/ level_intro_entry_1),
    JUMP(/*target*/ level_script_entry),
#endif
};
