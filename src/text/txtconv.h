#ifndef TXTDATA
#define TXTDATA

#include "types.h"

struct Character{
    char * txt;
    s32 value;
};

extern struct Character charmap[340];

u8 * getTranslatedText(char * txt);

#endif