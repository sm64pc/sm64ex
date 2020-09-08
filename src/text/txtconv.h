#ifndef TXTDATA
#define TXTDATA

#include "types.h"
struct Character{
    char * txt;
    s32 value[2];
};

u8 * getTranslatedText(char * txt);

#endif