#ifndef TXTDATA
#define TXTDATA

#include "types.h"

struct Character{
    char txt[3];
    s32 value;
};

extern struct Character charmap[340];

u8 * getTranslatedText(char * txt);

#endif