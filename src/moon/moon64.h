#ifndef MOON_WRAPPER
#define MOON_WRAPPER
#ifndef __cplusplus

#include "types.h"

void moon_init_languages(char *executable, char *gamedir);
u8 * moon_language_get_key( char* key );
void moon_set_language( int id );
const char* moon_get_language_name( int id );

#endif
#endif