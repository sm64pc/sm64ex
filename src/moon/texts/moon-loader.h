#ifndef MOON_TEXT_LOADER
#define MOON_TEXT_LOADER
#ifdef __cplusplus
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
    u8* * courses;
    std::map<std::string, u8*> strings;    
    std::vector<u8*> acts;
    std::vector<struct DialogEntry*> dialogs;
};

extern std::vector<LanguageEntry*> languages;

void Moon_LoadLanguage( std::string path );
void Moon_InitLanguages( char *exePath, char *gamedir ) ;
u8 * Moon_GetKey(std::string key);
void Moon_SetLanguage(LanguageEntry *new_language);

#endif
#endif