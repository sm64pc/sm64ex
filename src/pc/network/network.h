#ifndef NETWORK_H
#define NETWORK_H

#include <types.h>
#include "../cliopts.h"

void network_init(enum NetworkType networkType);
void network_track(struct MarioState *marioStates);
void network_send(char* buffer, int buffer_length);
void network_update(void);
void network_shutdown(void);

#endif
