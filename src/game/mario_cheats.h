#ifndef MARIO_CHEATS_H
#define MARIO_CHEATS_H

#include <PR/ultratypes.h>

#include "macros.h"
#include "types.h"

void cheats_set_model(struct MarioState *m);
void cheats_swimming_speed(struct MarioState *m);
void cheats_air_step(struct MarioState *m);
void cheats_long_jump(struct MarioState *m);
void cheats_mario_inputs(struct MarioState *m);

#endif // MARIO_CHEATS_H
