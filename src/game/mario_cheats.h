#ifndef MARIO_CHEATS_H
#define MARIO_CHEATS_H

#include <PR/ultratypes.h>

#include "macros.h"
#include "model_ids.h"
#include "types.h"

#if defined(MODEL_PLAYER) && defined(MODEL_LUIGIS_CAP)
#define R96
#endif

void cheats_set_model(struct MarioState *m);
void cheats_swimming_speed(struct MarioState *m);
void cheats_air_step(struct MarioState *m);
void cheats_long_jump(struct MarioState *m);
void cheats_mario_inputs(struct MarioState *m);

/* Options */
#define TIME_BUTTON 0x0080

#include "data/dynos.c.h"
#define __chaos_mode__   dynos_opt_get_value("chaos_mode")
#define __time_button__ dynos_opt_get_value("time_button")
#define __spl__ dynos_opt_get_value("spl")
#define __no_heavy__ dynos_opt_get_value("no_heavy")
#define __haz_walk__ dynos_opt_get_value("haz_walk")
#define __swim_any__ dynos_opt_get_value("swim_any")
#define __coin_mag__ dynos_opt_get_value("coin_mag")
#define __wat_con__ dynos_opt_get_value("wat_con")
#define __wat_lev__ dynos_opt_get_value("wat_lev")
#define CHAOS_MODE (__chaos_mode__ == 1)
#define SPL (__spl__)
#define NO_HEAVY (__no_heavy__ == 1)
#define HAZ_WALK (__haz_walk__ == 1)
#define SWIM_ANY (__swim_any__ == 1)
#define COIN_MAG (__coin_mag__ == 1)
#define WAT_CON (__wat_con__ == 1)
#define WAT_LEV (__wat_lev__)

#endif // MARIO_CHEATS_H
