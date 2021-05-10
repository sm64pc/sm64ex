#ifndef MOON_WRAPPER
#define MOON_WRAPPER

#ifndef __cplusplus

#include "types.h"
#include "pc/gfx/gfx_pc.h"

void moon_setup(char *state);

void moon_environment_save(char* key, char* value);

/*
#######################
    Moon Languagues
#######################
*/

u8 *moon_language_get_key( char* key );

/*
#######################
        Moon UI
#######################
*/

void moon_draw_ui();
void moon_ui_toggle();
void moon_change_ui(int index);
u8 moon_ui_open();

/*
#######################
        Moon IO
#######################
*/

void moon_update_window(void* window);

/*
######################
    Moon Texture
######################
*/

void moon_save_texture(struct TextureData* data, char* tex);
struct TextureData* moon_get_texture(char* tex);
struct TextureData* moon_create_texture();
void moon_load_base_texture(char* data, long size, char* texture);
void moon_load_texture(int tile, const char *fullpath, struct GfxRenderingAPI *gfx_rapi);

#endif
#endif