#ifndef MoonTextConverter
#define MoonTextConverter

#include "types.h"

extern u8 COLORED_MULTIPLY[];
extern u8 COLORED_MARIO_HEAD[];
extern u8 COLORED_COIN[];
extern u8 COLORED_STAR[];
extern u8 REGULAR_COIN[];
extern u8 REGULAR_MARIO_HEAD [];
extern u8 REGULAR_STAR_FILLED[];
extern u8 REGULAR_STAR_HOLLOW[];

#ifdef __cplusplus
#include <string>
namespace MoonInternal {
    void setupTextConverter(std::string state);
}

namespace Moon {
    uint8_t* GetTranslatedText(std::wstring in);
    uint8_t* GetTranslatedText(std::string in);
}
#else
u8* getTranslatedText( char* text );
#endif

#endif