#include "moon/mod-engine/textures/mod-texture.h"
#include "moon/io/modules/mouse-io.h"
#include "moon/ui/moon-ui-manager.h"
#include "moon/texts/moon-loader.h"
#include "moon/io/moon-io.h"

#include "moon/utils/moon-env.h"
#include "moon/mod-engine/engine.h"
#include "moon/mod-engine/test.h"
#include "moon/io/moon-io.h"

#include <iostream>
#include "moon/libs/nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;

extern "C" {
#include "game/game_init.h"
#include "types.h"

void moon_setup(char *state){
    MoonInternal::setupModEngine(string(state));
    MoonInternal::setupLanguageEngine(string(state));
    MoonInternal::setupIOModuleEngine(string(state));
}

/*
########################
    Moon Environment
########################
*/

void moon_environment_save(char* key, char* value){
    MoonInternal::saveEnvironmentVar(string(key), string(value));
}

/*
#######################
    Moon Languagues
#######################
*/

u8 * moon_language_get_key( char* key ){
    return getTranslatedText(Moon::getLanguageKey(std::string(key)).c_str());
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

void moon_update_window(void* window){
    MouseIO* tmp = Moon::GetIOModule<MouseIO>();
    if(tmp != NULL) tmp->window = window;
}

/*
######################
    Moon Texture
######################
*/

void moon_save_texture(struct TextureData* data, char* tex){
    MoonInternal::saveTexture(data, string(tex));
}

struct TextureData* moon_get_texture(char* tex){
    return Moon::getCachedTexture(string(tex));
}

struct TextureData* moon_create_texture(){
    return new TextureData();
}

void moon_load_base_texture(char* data, long size, char* texture){
    Moon::precacheBaseTexture(data, size, string(texture));

}

void moon_load_texture(int tile, const char *fullpath, struct GfxRenderingAPI *gfx_rapi){
    MoonInternal::loadTexture(tile, fullpath, gfx_rapi);
}

}