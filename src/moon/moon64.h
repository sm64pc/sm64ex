#ifndef MOON_WRAPPER
#define MOON_WRAPPER
#ifndef __cplusplus

#include "types.h"
#include "pc/gfx/gfx_pc.h"

void moon_init_languages(char *executable, char *gamedir);
u8 * moon_language_get_key( char* key );
void moon_set_language( int id );
const char* moon_get_language_name( int id );

void moon_draw_ui();
void moon_change_ui(int index);
u8   moon_ui_open();
void moon_ui_toggle();

void moon_modules_init();
void moon_modules_update();
void moon_update_window(void* window);

void moon_mod_engine_preinit();
void moon_mod_engine_init(char *executable, char *gamedir);

void moon_engine_save_texture(struct TextureData* data, char* tex);
struct TextureData* moon_engine_get_texture(char* tex);
struct TextureData* moon_engine_init_texture();
void moon_load_base_texture(char* data, long size, char* texture);

#endif
#endif