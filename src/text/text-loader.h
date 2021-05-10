#ifndef TXTLOADER
#define TXTLOADER

#include "types.h"
#include "game/ingame_menu.h"

extern u8 languagesAmount;

extern struct DialogEntry* * dialogPool;
extern u8* * seg2_course_name_table;
extern u8* * seg2_act_name_table;

struct StringTable {
    char* key;
    u8* value;
};

extern void         alloc_dialog_pool(char* exePath, char* gamedir);
extern void         dealloc_dialog_pool(void);
extern u8*          get_key_string(char* id);
#endif