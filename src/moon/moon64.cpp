#include "moon/texts/moon-loader.h"
#include "moon/network/moon-consumer.h"
#include <iostream>

extern "C" {

#include "types.h"

void moon_init_languages(char *executable, char *gamedir) {
    MoonConsumer consumer;
    consumer.Init();

    MoonResponse res;
    MoonRequest req;
    req.url = "https://raw.githubusercontent.com/Render96/Render96ex_Languages/master/PT_br.json";
    req.file = "/home/alex/testout/Kalimba2.txt";
    consumer.Get(req, &res);
    printf("%s\n", res.body.c_str());
    
    Moon_InitLanguages(executable, gamedir);
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