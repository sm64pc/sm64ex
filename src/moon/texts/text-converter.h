#ifndef MoonTextConverter
#define MoonTextConverter
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