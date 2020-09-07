#ifndef TXTLOADER
#define TXTLOADER

#include "types.h"
#include "game/ingame_menu.h"

extern char* currentLanguage;
extern s8 languagesAmount;

extern struct DialogEntry* * dialogPool;
extern struct LanguageEntry* * languages;

extern char* read_file(char* name);

struct LanguageEntry {
    char * name;
    char * logo;
    struct DialogEntry* * entries;
};

extern struct LanguageEntry* get_language_by_name(char* name);
extern struct LanguageEntry* get_language();
extern void set_language(struct LanguageEntry* new_language);
extern void alloc_dialog_pool(void);
#endif