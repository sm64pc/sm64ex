#include "moon/texts/moon-loader.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/io/moon-io.h"
#include <iostream>

#include "moon/mod-engine/engine.h"
#include "moon/io/moon-io.h"
#include "moon/io/modules/mouse-io.h"
#include "moon/mod-engine/test.h"
#include "moon/libs/nlohmann/json.hpp"
using json = nlohmann::json;

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
    // Moon_InitModEngine();
    // exit(0);
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

/*
#######################
        Moon IO
#######################
*/

void moon_modules_init(){
    InitIOModules();
}
void moon_modules_update(){
    UpdateIOModules();
}
void moon_update_window(void* window){
    MouseIO* tmp = GetIOModule<MouseIO>();
    if(tmp != NULL) tmp->window = window;
}

void moon_mod_engine_preinit(){
    Moon_PreInitModEngine();
}

void moon_mod_engine_init(char *executable, char *gamedir){
    Moon_InitModEngine(executable, gamedir);
}

void moon_engine_save_texture(struct TextureData* data, char* tex){
    Moon_SaveTexture(data, string(tex));
}

struct TextureData* moon_engine_get_texture(char* tex){
    return Moon_GetTexture(string(tex));
}

struct TextureData* moon_engine_init_texture(){
    return new TextureData();
}

void moon_load_base_texture(char* data, long size, char* texture){
    Moon_LoadBaseTexture(data, size, string(texture));
}

}