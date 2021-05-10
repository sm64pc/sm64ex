#ifndef MOON_TEXT_LOADER
#define MOON_TEXT_LOADER

#include <vector>
#include <list>
#include <map>
#include <string>
#include <codecvt>
#include <locale>
#include "moon/libs/rapidjson/filereadstream.h"
#include "moon/libs/rapidjson/encodings.h"
#include "moon/libs/rapidjson/document.h"

extern "C" {
    #include "types.h"
    #include "game/ingame_menu.h"
    #include "text/txtconv.h"
}

inline std::wstring wide (const std::string& str) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
}

inline std::string narrow (const std::wstring& str) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes( str );
}

struct LanguageEntry {
    std::string name;
    std::string logo;
    std::map<std::string, std::string> strings;
    std::vector<u8*> acts;
    std::vector<struct DialogEntry*> dialogs;
    std::vector<u8*> courses;
};

namespace Moon {
    extern std::vector<LanguageEntry*> languages;
    extern LanguageEntry *current;

    void loadLanguage(std::string path);
    void setCurrentLanguage(LanguageEntry *new_language);
    std::string getLanguageKey(std::string key);
}

namespace MoonInternal {
    void scanLanguagesDirectory();
    void setupLanguageEngine( std::string state );
}

#endif