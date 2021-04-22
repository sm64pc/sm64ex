#include "moon/texts/moon-loader.h"
#include <iostream>

extern "C" {

#include "types.h"

void moon_init_languages(char *executable, char *gamedir) {
    printf("---------------------------------\n");
    Moon_InitLanguages(executable, gamedir);
    printf("---------------------------------\n");
}

u8 * moon_language_get_key( char* key ){
    return Moon_GetKey(std::string(key));
}

void moon_set_language( int id ) {
    Moon_SetLanguage(languages[id]);
}

const char* moon_get_language_name( int id ) {
    return languages[id]->name.c_str();
}

}