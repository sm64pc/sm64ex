#ifndef TXTLOADER
#define TXTLOADER

#include "types.h"
#include "game/ingame_menu.h"

extern char* currentLanguage;
extern s8 languagesAmount;

extern struct DialogEntry ** dialogPool;
extern char* read_file(char* name);

struct LanguageEntry {
    char * name;
    char * logo;
    char * placeholder;
    struct DialogEntry* * entries;
};

extern void alloc_dialog_pool(void);
#endif