#include "moon/texts/moon-loader.h"
#include "moon/ui/moon-ui-manager.h"
#include <iostream>

extern "C" {

#include "game/game_init.h"
#include "types.h"

/*
#######################
    Moon Languagues
#######################
*/

void moon_init_languages(char *executable, char *gamedir) {
    Moon_InitLanguages(executable, gamedir);
}

u8 * moon_language_get_key( char* key ){
    return getTranslatedText(Moon_GetKey(std::string(key)).c_str());
}

void moon_set_language( int id ) {
    Moon_SetLanguage(languages[id]);
}

const char* moon_get_language_name( int id ) {
    return languages[id]->name.c_str();
}

/*
#######################
        Moon UI
#######################
*/

void moon_draw_ui(){
    MoonDrawUI();
}

void moon_ui_toggle(){
    MoonHandleToggle();
}

void moon_change_ui(int index){
    MoonChangeUI(index);
}

u8 moon_ui_open(){
    return isOpen;
}

}