#ifndef MoonTextConverter
#define MoonTextConverter

#include <string>

namespace MoonInternal {
    void setupTextConverter(std::string state);
}

namespace Moon {
    uint8_t* GetTranslatedText(std::wstring in);
    uint8_t* GetTranslatedText(std::string in);
}
#endif